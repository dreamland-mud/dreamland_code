/* $Id$
 *
 * ruffina, 2009
 */
#include "language.h"
#include "languagemanager.h"
#include "word.h"
#include "wordeffect.h"
#include "xmlattributelanguage.h"
#include "fenia/exceptions.h"
#include "fenia/register-impl.h"
#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "regcontainer.h"

#include "commandmanager.h"
#include "skillreference.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "act.h"
#include "mudtags.h"
#include "loadsave.h"
#include "wiznet.h"

#include "def.h"
#include "l10n.h"

GSN(garble);
GSN(deafen);

void LanguageCommand::setLanguage(Language::Pointer language)
{
    this->language = language;

    commandManager->registrate( Pointer( this ) );
}

Language::Pointer LanguageCommand::getLanguage() const
{
    return language;
}

void LanguageCommand::unsetLanguage()
{
    commandManager->unregistrate( Pointer( this ) );
    
    language.clear();
}

bool LanguageCommand::saveCommand() const
{
    if (language)
        return languageManager->saveXML(*language, language->getName());

    return false;
}

void LanguageCommand::run( Character *ach, const DLString &constArguments )
{
    DLString arguments( constArguments );
    DLString arg;
    PCharacter *ch;

    arg = arguments.getOneArgument( );
    ch = ach->getPC( );

    if (!ch) {
        ach->pecho( _("Муу-у-у.") );
        return;
    }

    // Words arrive in dreams (LanguageManager::run fires only at POS_SLEEPING), so
    // the command's position is 'sleep' for the sake of reading that list back
    // without waking up. Nothing else works with your eyes shut -- uttering a word
    // in your sleep would hand out its effect for free.
    if (!IS_AWAKE(ch) && !arg_is_list( arg )) {
        ch->pecho( _("Во сне? Или может сначала проснешься...") );
        return;
    }

    if (arg.empty( )) {
        ch->pecho( _("Что ты хочешь произнести на %^N6?"), language->getNameFor(ch).c_str( ) );
        return;
    }
    
    if (arg_is_all( arg ) && ch->is_immortal( )) {
        doList( ch );
        return;
    }

    if (arg_is(arg, "init") && ch->isCoder( )) {
        doInit( ch, arguments );
        return;
    }
    
    if (!available( ch )) {
        ch->pecho( _("Ты не умеешь разговаривать на %^N6."), language->getNameFor(ch).c_str( ) );
        return;
    }

    if (arg_is_list( arg )) {
        doKnown( ch );
        return;
    }
        
    if (arg_is(arg, "sense")) {
        doIdent( ch, arguments );
        return;
    }

    if (arg_is(arg, "forget")) {
        doForget( ch, arguments );
        return;
    }

    if (arg_is(arg, "remember")) {
        doRemember( ch, arguments );
        return;
    }
    
    doUtter( ch, arg, arguments );
}

static void locateTargets(WordEffect::Pointer effect, PCharacter *ch, Character *&victim, Object *&obj, const DLString &arg2)
{
    if (effect && effect->isObject()) {
        obj = get_obj_here(ch, arg2);
        return;
    }

    if (arg2.empty()) {
        if (effect && effect->isOffensive()) 
            victim = ch->fighting;
        return;
    }
    
    victim = get_char_room(ch, arg2);
}

// Give a Fenia handler first crack at a word-effect. .WordEffect(lang,name).runObj
// (object target) or .runVict (character target) fully replaces the C++ effect when
// defined, and its boolean return becomes the "word consumed" flag. Returns false
// with fUsed untouched when no Fenia handler is present, so the C++ effect runs as
// before -- this dispatch is additive and shared by all five languages.
static bool runFeniaEffect( WordEffect::Pointer effect, Character *ch, Object *obj, Character *victim, bool &fUsed )
{
    WrapperBase *base = effect->getWrapper( );
    if (!base)
        return false;

    if (obj) {
        static Scripting::IdRef runObjId( "runObj" );
        if (!base->hasTrigger( "runObj" ))
            return false;
        fUsed = base->call( runObjId, "CO", ch, obj );
        return true;
    }

    static Scripting::IdRef runVictId( "runVict" );
    if (!base->hasTrigger( "runVict" ))
        return false;
    fUsed = base->call( runVictId, "CC", ch, victim );
    return true;
}

// Hand a doUtter outcome to a Fenia handler under .tmp.language so the feedback
// is trilingual and hot-reloadable instead of the hardcoded C++ lines. Shared by
// all four languages; `method` selects the handler and `tail` is its 6th arg:
//   "onOutcome" -- an uttered word with no runnable effect (tail noword/nopower);
//   "onEcho"    -- the self/victim/bystander echoes and the pronunciation-fail
//                  lines (tail echo/garble).
// Returns true when Fenia owned the output, so the caller skips its C++ fallback.
// Additive: a missing module or any error leaves the fallback to C++.
static bool runFeniaHook( Character *ch, const DLString &langName, const DLString &wordStr,
                          Object *obj, Character *victim, const char *tail, const char *method )
{
    using namespace Scripting;

    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP( "tmp" ), ID_LANGUAGE( "language" );
    IdRef methodId( method );

    try {
        Register tmp = *Context::root[ID_TMP];
        Register lng = *tmp[ID_LANGUAGE];
        Register fn  = *lng[methodId];

        if (fn.type != Register::FUNCTION)
            return false;

        RegisterList args;
        args.push_back( FeniaManager::wrapperManager->getWrapper( ch ) );
        args.push_back( Register( langName ) );
        args.push_back( Register( wordStr ) );
        args.push_back( obj ? FeniaManager::wrapperManager->getWrapper( obj ) : Register( ) );
        args.push_back( victim ? FeniaManager::wrapperManager->getWrapper( victim ) : Register( ) );
        args.push_back( Register( DLString( tail ) ) );

        Register rc = fn.toFunction( )->invoke( lng, args );
        return rc.toBoolean( );

    } catch (const ::Exception &e) {
        FeniaManager::getThis( )->croak( 0, Register( DLString( "language." ) + method ), e );
    }

    return false;
}

// Hand a doUtter command-layer *check* to a Fenia handler under .tmp.language.onCheck
// so the design roll is tunable and hot-reloadable. Unlike runFeniaHook (which emits
// output and returns "did Fenia handle it"), this returns the check's boolean verdict;
// `handled` reports whether a Fenia handler actually answered, so the caller can fall
// back to its own C++ roll when the module is absent. `chance` is the speaker's
// effective language-skill %. Additive and shared by all four languages: a missing
// module or any error leaves the decision to C++.
static bool runFeniaCheck( Character *ch, const DLString &langName, const DLString &wordStr,
                           const char *kind, int chance, bool &handled )
{
    using namespace Scripting;

    handled = false;

    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP( "tmp" ), ID_LANGUAGE( "language" ), ID_ONCHECK( "onCheck" );

    try {
        Register tmp = *Context::root[ID_TMP];
        Register lng = *tmp[ID_LANGUAGE];
        Register fn  = *lng[ID_ONCHECK];

        if (fn.type != Register::FUNCTION)
            return false;

        RegisterList args;
        args.push_back( FeniaManager::wrapperManager->getWrapper( ch ) );
        args.push_back( Register( langName ) );
        args.push_back( Register( wordStr ) );
        args.push_back( Register( DLString( kind ) ) );
        args.push_back( Register( chance ) );

        Register rc = fn.toFunction( )->invoke( lng, args );
        // Read the verdict BEFORE marking handled, so a malformed future handler --
        // one whose return makes toBoolean() throw (a NONE/OBJECT/FUNCTION value) --
        // fails safe to the C++ roll via the catch below, the way the sibling
        // runFeniaHook does, instead of leaving handled=true and silently skipping
        // the fallback (never garbling). The shipped "garble" handler returns a bool,
        // so this path never throws today; the guard is for later onCheck kinds.
        bool verdict = rc.toBoolean( );
        handled = true;
        return verdict;

    } catch (const ::Exception &e) {
        FeniaManager::getThis( )->croak( 0, Register( DLString( "language.onCheck" ) ), e );
    }

    return false;
}

void LanguageCommand::doUtter( PCharacter *ch, DLString &arg1, DLString &arg2 ) const
{
    Character *rch, *victim;
    Object *obj;
    int chance;
    bool fMiss, fUsed;
    Word word;
    WordContainer *wcontainer;
    WordEffect::Pointer effect;
    
    if (!language->usable( ch, true ))
        return;
    
    chance = language->getEffective( ch );

    // The pronunciation/garble roll -- the last command-layer design check that was
    // still hardcoded here. Fenia (.tmp.language.onCheck, kind "garble") owns the
    // verdict when the module is present, so the difficulty is hot-reloadable; C++
    // keeps its own roll as the fallback. The gsn_garble curse forces a garble
    // regardless and is deliberately not Fenia's call. The garble *output* is already
    // Fenia (runFeniaHook onEcho, below).
    bool checkHandled = false;
    bool garbled = runFeniaCheck( ch, language->getName( ), arg1, "garble", chance, checkHandled );
    if (!checkHandled)
        garbled = number_percent( ) > chance;
    if (ch->isAffected( gsn_garble ))
        garbled = true;

    if (garbled) {
        if (!runFeniaHook( ch, language->getName( ), arg1, 0, 0, "garble", "onEcho" )) {
            ch->pecho( _("Тебя подвело произношение.") );
            ch->recho( POS_RESTING, _("%^C1 бормочет что-то неразборчивое."), ch );
        }
        ch->setWait( language->getBeats(ch) / 2 );
        return;
    }

    // Neutralize player-injected {h command tags in the uttered token before it is
    // echoed to the targeted player and to bystanders -- same injection class the
    // channel strip closes, but this command is off that framework.
    mudtags_strip_web( arg1, ch );

    obj = NULL;
    victim = NULL;
    fMiss = false;

    wcontainer = language->locateWord( word, ch, arg1 );
    effect = word.getEffect( );
    locateTargets(effect, ch, victim, obj, arg2);
    fMiss = effect && !effect->isObject() && !victim;
         
    // Phase 1 M2: delegate the self/victim/bystander echoes to Fenia
    // (.tmp.language.onEcho) for trilingual, hot-reloadable output. The C++
    // block below is kept verbatim as the fallback for when the module is absent.
    if (!runFeniaHook( ch, language->getName( ), arg1, obj, victim, "echo", "onEcho" )) {
    if (obj) {
        if (number_bits( 1 ))
            ch->pecho( _("Ты проводишь рукой над %O5 и изрекаешь '{C%s{x'."), obj, arg1.c_str( ) );
        else
            ch->pecho( _("Ты изрекаешь, глядя на %O4: '{C%s{x'."), obj, arg1.c_str( ) );
    }
    else if (!victim || victim == ch)
        ch->pecho( _("Ты изрекаешь '{C%s{x'"), arg1.c_str( ) );
    else {
        ch->pecho( _("Ты изрекаешь, указывая на %^C4: '{C%s{x'"), victim, arg1.c_str( ) );

        if (IS_AWAKE(victim) && !victim->isAffected( gsn_deafen )) {
            if (language->getEffective( victim ) < number_percent( ))
                victim->pecho( _("%^C1 что-то произносит, указывая в твою сторону."), ch );
            else
                victim->pecho( _("%^C1 изрекает на %^N6, указывая в твою сторону: '{C%s{x'"),
                               ch, language->getNameFor(victim).c_str( ), arg1.c_str( ) );
        }
    }

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (!IS_AWAKE(rch))
            continue;

        if (rch == ch || rch == victim)
            continue;

        if (rch->isAffected( gsn_deafen ))
            continue;

        if (language->getEffective( rch ) < number_percent( ))
            rch->pecho( _("%^C1 что-то бормочет на странном языке."), ch );
        else
            rch->pecho( _("%^C1 изрекает на %^N6 '{C%s{x'"),
                        ch, language->getNameFor(rch).c_str( ), arg1.c_str( ) );
    }
    }

    if (word.empty( ) || !effect) {
        const char *outcome = word.empty( ) ? "noword" : "nopower";
        if (!runFeniaHook( ch, language->getName( ), arg1, obj, victim, outcome, "onOutcome" ))
            oldact(_("{CНа мгновение все вокруг стихло.{x"), ch, 0, 0, TO_ALL );
        return;
    }

    if (effect->isObject( ) && !obj) {
        if (!runFeniaHook( ch, language->getName( ), arg1, obj, victim, "needobject", "onOutcome" ))
            ch->pecho(_("Выбери, на какую вещь произнести слово."));
        fUsed = false;
    }
    else if (obj) {
        if (!runFeniaEffect( effect, ch, obj, NULL, fUsed ))
            fUsed = effect->run( ch, obj );
    }
    else {
        if (fMiss) {
            if (!runFeniaHook( ch, language->getName( ), arg1, obj, victim, "miss", "onOutcome" ))
                ch->pecho( _("Твои слова, не достигнув цели, обратились на тебя сам%Gого|ого|у."), ch );
        }

        Character *tgt = (!victim ? ch : victim);
        if (!runFeniaEffect( effect, ch, NULL, tgt, fUsed ))
            fUsed = effect->run( ch, tgt );
    }
    
    if (fUsed) {
        wcontainer->wordUsed( word, ch );
        language->improve( ch, true );
        ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" )->addWord(word, true);

        wiznet( WIZ_LANGUAGE, 0, 0, "%^C1 изрекает слово '%s' (%s) на %s.", 
                ch, word.toStr( ), word.effect.getValue( ).c_str( ),
                (obj ? obj->getShortDescr( '4', LANG_DEFAULT ).c_str( ) :
                       !victim || victim == ch ? "себя" : 
                                                 victim->getNameP( '4' ).c_str( ) ));
    }

    ch->setWait( language->getBeats(ch) );
}

void LanguageCommand::doList( PCharacter *ch ) const
{
    const LanguageManager::Words &words = languageManager->getWords( );
    LanguageManager::Words::const_iterator i;
    
    ch->pecho( _("Текущий словарный запас для языка {c%N1{x: "), language->getNameFor(ch).c_str( ) );
        
    for (i = words.begin( ); i != words.end( ); i++) {
        const Word & w = i->second;

        if (w.lang.getValue( ) == getName())
            ch->pecho( "   %-20s   [%1d]  [%2d]  %N1",
                             w.dictum.getValue( ).c_str( ),
                             w.count.getValue( ),
                             w.getPower( ),
                             w.effect.getValue( ).c_str( ) );
    }
}

void LanguageCommand::doInit( PCharacter *ch, DLString &arg ) const
{
    languageManager->eraseWords( language );
}

void LanguageCommand::doKnown( PCharacter *ch ) const
{
    bool hasDreams = showDreams( ch );
    bool hasRewards = showRewards( ch );
    if (!hasDreams && !hasRewards)
        ch->pecho( _("Ты не можешь вспомнить ни одного слова.") );
}

/*
 * Display list of words seen in a dream.
 */
bool LanguageCommand::showDreams( PCharacter *ch ) const
{
    WordEffect::Pointer ef;
    XMLAttributeLanguage::Pointer attr;
    XMLAttributeLanguageHints::Pointer attrHints;
    XMLAttributeLanguage::Words::iterator w;;

    attr = ch->getAttributes( ).findAttr<XMLAttributeLanguage>( "language" );
    attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );
    
    if (!attr) {
//        ch->pecho( "Тебе ни разу ничего не снилось." );
        return false;
    }
        
    XMLAttributeLanguage::Words &words = attr->getWords( );

    if (words.empty( )) {
//        ch->pecho( "Все сны давно забылись." );
        return false;
    }
    
    ostringstream buf;

    for (w = words.begin( ); w != words.end( ); w++) {
        Language::Pointer wordLang = languageManager->findLanguage( w->second.lang );
        if (!wordLang)
            continue;

        if (wordLang->getName( ) != getName( ))
            continue;

        bool fShowEffect = (wordLang->getLearned( ch ) == Language::SKILL_NATIVE);
        bool hasHint = attrHints && attrHints->hasHint( w->second );

        buf << fmt(0, "        {c%-30s{x", w->second.dictum.getValue( ).c_str( ) );
        
        ef = w->second.getEffect( );

        if (ef && (fShowEffect || hasHint )) 
            buf << fmt( 0, "    (%N1)", ef->getMeaning( viewerLang(ch) ).c_str( ) );
        buf << endl;
    }

    if (buf.str( ).empty( )) {
        ch->pecho( _("Тебе ни разу ничего не снилось на %N6."), language->getNameFor(ch).c_str( ) );
        return true;
    }

    buf << endl;
    ch->pecho( _("Тебе приснились и запомнились слова на %N6: "), language->getNameFor(ch).c_str( ) );
    ch->send_to( buf );
    return true;
}

/*
 * Display words obtained as a quest reward. 
 */
bool LanguageCommand::showRewards( PCharacter *ch ) const
{
    XMLAttributeLanguage::Pointer attr;
    XMLAttributeLanguageHints::Pointer attrHints;
    XMLAttributeLanguageHints::Hints::const_iterator h;

    attr = ch->getAttributes( ).findAttr<XMLAttributeLanguage>( "language" );
    attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );

    if (!attrHints) {
        return false;
    }
    
    ostringstream buf;
    std::list<DLString> expiredWords;

    for (h = attrHints->hints.begin( ); h != attrHints->hints.end( ); h++) {
        const DLString &dictum = h->first;
        bool hasHint = h->second;
        
        // Hide words from the 'dream list', as they can be part of hints too.
        if (attr) {
            Word dreamedWord;
            if (attr->findWord( dreamedWord, dictum ))
                continue;
        }

        // Resolve the word. Remember expired ones for further deletion.
        Word rewardWord;
        if (!languageManager->findWord( rewardWord, dictum )) {
            buf << fmt(0, "        {D%-30s{x\r\n", dictum.c_str( ) );
            expiredWords.push_back( dictum );
            continue;
        }
            
        LanguagePointer lang = languageManager->findLanguage( rewardWord.lang );
        if (!lang)
            continue;

        bool fShowEffect = hasHint || (lang->getLearned( ch ) == Language::SKILL_NATIVE);
        WordEffect::Pointer ef = rewardWord.getEffect( );

        buf << fmt(0, "        {c%-30s{x", dictum.c_str( ) );

        if (ef && (fShowEffect || hasHint )) 
            buf << fmt( 0, "    (%N1)", ef->getMeaning( viewerLang(ch) ).c_str( ) );

        buf << endl;
    }
    
    for (std::list<DLString>::iterator e = expiredWords.begin( ); e != expiredWords.end( ); e++)
        attrHints->removeWord( *e );
    
    if (buf.str( ).empty( )) {
        return false;
    }

    ch->pecho( _("Тебе сообщили такие слова на разных языках: ") );
    ch->send_to( buf );
    return true;
}

void LanguageCommand::doIdent( PCharacter *ch, DLString &arguments ) const
{
    int chance;
    WordEffect::Pointer ef;
    Word word;
    DLString arg = arguments.getOneArgument();
    
    if (arg.empty( )) {
        ch->pecho( _("Смысл чего ты пытаешься понять?") );
        return;
    }
    
    if (ch->isCoder())
        chance = 100;
    else
        chance = language->getLearned( ch );
    
    if (chance < Language::SKILL_SENSE || number_percent( ) > chance) {
        ch->pecho( _("Тайный смысл слов %^N2 ускользает от тебя."), language->getNameFor(ch).c_str( ) );
        return;
    }

    if (!language->locateWord( word, ch, arg ) || !( ef = word.getEffect( ) )) {
        ch->pecho( _("Звучание слова %s кажется тебе бессмысленным."), arg.c_str( ) );
        return;
    }

    ch->pecho( _("Знание языка помогает тебе приподнять завесу тайны над словом '{c%s{x':"), arg.c_str( ) );
    ch->pecho( _("оно содержит в себе секрет {c%N2{x."), ef->getMeaning( viewerLang(ch) ).c_str( ) );
    wiznet( WIZ_LANGUAGE, 0, 0, "%^C1 узнает смысл слова '%s' (%s).", 
            ch, word.toStr( ), word.effect.getValue( ).c_str( ) );

    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" );
    attrHints->addWord( word, true );
}

void LanguageCommand::doForget( PCharacter *ch, const DLString &arg ) const
{
    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );
    XMLAttributeLanguage::Pointer attr = ch->getAttributes( ).findAttr<XMLAttributeLanguage>( "language" );

    if (arg.empty()) {
        ch->pecho(_("Что именно ты хочешь забыть?"));
        return;
    }

    if (!attr && !attrHints) {
        ch->pecho(_("Ты не знаешь никаких слов."));
        return;
    }

    if (arg_is_all(arg)) {
        // Clear all hints pertaining to the dreamed words.
        if (attrHints && attr) {
            for (XMLAttributeLanguage::Words::iterator w = attr->getWords( ).begin( ); w != attr->getWords( ).end( ); w++)
                attrHints->removeWord( w->first );
        }

        if (attr)
            attr->getWords( ).clear( );

        ch->pecho( _("Все приснившиеся тебе слова ускользают из твоей памяти.") );
        return;
    }

    // Find and forget a word among the dreams.    
    Word word;
    if (attr && attr->findWord(word, arg)) {
        attr->removeWord(word, ch);
        ch->pecho(_("Ты забываешь слово %s."), arg.c_str());
        return;
    }

    // Find and forget a word among the rewards or overheard ones.
    if (attrHints && attrHints->hasWord(arg)) {
        attrHints->removeWord(arg);
        ch->pecho(_("Ты забываешь слово %s."), arg.c_str());
        return;
    }

    ch->pecho(_("Ты не знаешь такого слова."));
}

void LanguageCommand::doRemember( PCharacter *ch, const DLString &arg ) const
{
    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" );

    if (arg.empty()) {
        ch->pecho(_("Что именно ты хочешь запомнить?"));
        return;
    }
    
    if (attrHints->hasWord(arg)) {
        ch->pecho(_("Ты и так помнишь это слово."));
        return;
    }
    
    Word word;
    if (!language->locateWord( word, ch, arg )) {
        ch->pecho(_("Это слово не существует."));
        return;
    }

    attrHints->addWord(word, false); 
    ch->pecho(_("Ты запоминаешь слово %s."), word.dictum.c_str());
}


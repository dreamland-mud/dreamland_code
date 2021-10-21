/* $Id$
 *
 * ruffina, 2009
 */
#include "language.h"
#include "languagemanager.h"
#include "word.h"
#include "wordeffect.h"
#include "xmlattributelanguage.h"

#include "skillreference.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "act.h"
#include "loadsave.h"
#include "wiznet.h"
#include "mercdb.h"
#include "def.h"

const Enumeration Language::defaultPosition( POS_RESTING, &position_table );

GSN(garble);
GSN(deafen);

const DLString & Language::getRussianName( ) const
{
    if (!nameRusNoCase.empty( ))
        return nameRusNoCase;
    else
        return nameRus;
}

const Flags & Language::getCommandCategory( ) const
{
    return cat;
}

void Language::run( Character *ach, const DLString &constArguments )
{
    DLString arguments( constArguments );
    DLString arg;
    PCharacter *ch;

    arg = arguments.getOneArgument( );
    ch = ach->getPC( );

    if (!ch) {
        ch->pecho( "Муу-у-у." );
        return;
    }

    if (arg.empty( )) {
        ch->pecho( "Что ты хочешь произнести на %^N6?", nameRus.getValue( ).c_str( ) );
        return;
    }
    
    if (arg_is_all( arg ) && ch->is_immortal( )) {
        doList( ch );
        return;
    }

    if (arg.strPrefix( "init" ) && ch->isCoder( )) {
        doInit( ch, arguments );
        return;
    }
    
    if (!available( ch )) {
        ch->pecho( "Ты не умеешь разговаривать на %^N6.", nameRus.getValue( ).c_str( ) );
        return;
    }

    if (arg_is_list( arg ) || arg.strPrefix( "known" )) {
        doKnown( ch );
        return;
    }
        
    if (arg_oneof( arg, "sense", "смысл" )) {
        doIdent( ch, arguments );
        return;
    }

    if (arg_oneof( arg, "forget", "забыть" )) {
        doForget( ch, arguments );
        return;
    }

    if (arg_oneof( arg, "remember", "запомнить" )) {
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

void Language::doUtter( PCharacter *ch, DLString &arg1, DLString &arg2 ) const
{
    Character *rch, *victim;
    Object *obj;
    int chance;
    bool fMiss, fUsed;
    Word word;
    WordContainer *wcontainer;
    WordEffect::Pointer effect;
    
    if (!usable( ch, true ))
        return;
    
    chance = getEffective( ch );
    
    if (number_percent( ) > chance || ch->isAffected( gsn_garble )) {
        ch->pecho( "Тебя подвело произношение." );
        ch->recho( POS_RESTING, "%^C1 бормочет что-то неразборчивое.", ch );
        ch->setWait( getBeats(ch) / 2 );
        return;
    }
    
    obj = NULL;
    victim = NULL;
    fMiss = false;

    wcontainer = locateWord( word, ch, arg1 );
    effect = word.getEffect( );
    locateTargets(effect, ch, victim, obj, arg2);
    fMiss = effect && !effect->isObject() && !victim;
         
    if (obj) {
        if (number_bits( 1 ))
            ch->pecho( "Ты проводишь рукой над %O5 и изрекаешь '{C%s{x'.", obj, arg1.c_str( ) );
        else
            ch->pecho( "Ты изрекаешь, глядя на %O4: '{C%s{x'.", obj, arg1.c_str( ) );
    }
    else if (!victim || victim == ch)
        ch->pecho( "Ты изрекаешь '{C%s{x'", arg1.c_str( ) );
    else {
        ch->pecho( "Ты изрекаешь, указывая на %^C4: '{C%s{x'", victim, arg1.c_str( ) );
        
        if (IS_AWAKE(victim) && !victim->isAffected( gsn_deafen )) {
            if (getEffective( victim ) < number_percent( ))
                victim->pecho( "%^C1 что-то произносит, указывая в твою сторону.", ch );
            else 
                victim->pecho( "%^C1 изрекает на %^N6, указывая в твою сторону: '{C%s{x'",
                               ch, nameRus.getValue( ).c_str( ), arg1.c_str( ) );
        }
    }

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (!IS_AWAKE(rch))
            continue;
        
        if (rch == ch || rch == victim)
            continue;

        if (rch->isAffected( gsn_deafen ))
            continue;

        if (getEffective( rch ) < number_percent( ))
            rch->pecho( "%^C1 что-то бормочет на странном языке.", ch );
        else 
            rch->pecho( "%^C1 изрекает на %^N6 '{C%s{x'", 
                        ch, nameRus.getValue( ).c_str( ), arg1.c_str( ) );
    }
    
    if (word.empty( ) || !effect) {
        oldact("{CНа мгновение все вокруг стихло.{x", ch, 0, 0, TO_ALL );
        return;
    }

    if (effect->isObject( ) && !obj) {
        ch->pecho("Выбери, на какую вещь произнести слово.");
        fUsed = false;
    }
    else if (obj) {
        fUsed = effect->run( ch, obj );
    }
    else {
        if (fMiss)
            ch->pecho( "Твои слова, не достигнув цели, обратились на тебя сам%Gого|ого|у.", ch );
        
        fUsed = effect->run( ch, (!victim ? ch : victim) );
    }
    
    if (fUsed) {
        wcontainer->wordUsed( word, ch );
        improve( ch, true );
        ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" )->addWord(word, true);
        wiznet( WIZ_LANGUAGE, 0, 0, "%^C1 изрекает слово '%s' (%s) на %s.", 
                ch, word.toStr( ), word.effect.getValue( ).c_str( ),
                (obj ? obj->getShortDescr( '4' ).c_str( ) :
                       !victim || victim == ch ? "себя" : 
                                                 victim->getNameP( '4' ).c_str( ) ));
    }

    ch->setWait( getBeats(ch) );
}

void Language::doList( PCharacter *ch ) const
{
    const LanguageManager::Words &words = languageManager->getWords( );
    LanguageManager::Words::const_iterator i;
    
    ch->pecho( "Текущий словарный запас для языка {c%N1{x: ", nameRus.getValue( ).c_str( ) );
        
    for (i = words.begin( ); i != words.end( ); i++) {
        const Word & w = i->second;

        if (w.lang.getValue( ) == name)
            ch->pecho( "   %-20s   [%1d]  [%2d]  %N1",
                             w.dictum.getValue( ).c_str( ),
                             w.count.getValue( ),
                             w.getPower( ),
                             w.effect.getValue( ).c_str( ) );
    }
}

void Language::doInit( PCharacter *ch, DLString &arg ) const
{
    languageManager->eraseWords( *this );
}

void Language::doKnown( PCharacter *ch ) const
{
    bool hasDreams = showDreams( ch );
    bool hasRewards = showRewards( ch );
    if (!hasDreams && !hasRewards)
        ch->pecho( "Ты не можешь вспомнить ни одного слова." );
}

/*
 * Display list of words seen in a dream.
 */
bool Language::showDreams( PCharacter *ch ) const
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

        bool fShowEffect = (wordLang->getLearned( ch ) == SKILL_NATIVE);
        bool hasHint = attrHints && attrHints->hasHint( w->second );

        buf << dlprintf( "        {c%-30s{x", w->second.dictum.getValue( ).c_str( ) );
        
        ef = w->second.getEffect( );

        if (ef && (fShowEffect || hasHint )) 
            buf << fmt( 0, "    (%N1)", ef->getMeaning( ).c_str( ) );
        buf << endl;
    }

    if (buf.str( ).empty( )) {
        ch->pecho( "Тебе ни разу ничего не снилось на %N6.", nameRus.getValue( ).c_str( ) );
        return true;
    }

    buf << endl;
    ch->pecho( "Тебе приснились и запомнились слова на %N6: ", nameRus.getValue( ).c_str( ) );
    ch->send_to( buf );
    return true;
}

/*
 * Display words obtained as a quest reward. 
 */
bool Language::showRewards( PCharacter *ch ) const
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
            buf << dlprintf( "        {D%-30s{x\r\n", dictum.c_str( ) );
            expiredWords.push_back( dictum );
            continue;
        }
            
        LanguagePointer lang = languageManager->findLanguage( rewardWord.lang );
        if (!lang)
            continue;

        bool fShowEffect = hasHint || (lang->getLearned( ch ) == SKILL_NATIVE);
        WordEffect::Pointer ef = rewardWord.getEffect( );

        buf << dlprintf( "        {c%-30s{x", dictum.c_str( ) );

        if (ef && (fShowEffect || hasHint )) 
            buf << fmt( 0, "    (%N1)", ef->getMeaning( ).c_str( ) );

        buf << endl;
    }
    
    for (std::list<DLString>::iterator e = expiredWords.begin( ); e != expiredWords.end( ); e++)
        attrHints->removeWord( *e );
    
    if (buf.str( ).empty( )) {
        return false;
    }

    ch->pecho( "Тебе сообщили такие слова на разных языках: " );
    ch->send_to( buf );
    return true;
}

void Language::doIdent( PCharacter *ch, DLString &arguments ) const
{
    int chance;
    WordEffect::Pointer ef;
    Word word;
    DLString arg = arguments.getOneArgument();
    
    if (arg.empty( )) {
        ch->pecho( "Смысл чего ты пытаешься понять?" );
        return;
    }
    
    if (ch->isCoder())
        chance = 100;
    else
        chance = getLearned( ch );
    
    if (chance < SKILL_SENSE || number_percent( ) > chance) {
        ch->pecho( "Тайный смысл слов %^N2 ускользает от тебя.", nameRus.getValue( ).c_str( ) );
        return;
    }

    if (!locateWord( word, ch, arg ) || !( ef = word.getEffect( ) )) {
        ch->pecho( "Звучание слова %s кажется тебе бессмысленным.", arg.c_str( ) );
        return;
    }

    ch->pecho( "Знание языка помогает тебе приподнять завесу тайны над словом '{c%s{x':", arg.c_str( ) );
    ch->pecho( "оно содержит в себе секрет {c%N2{x.", ef->getMeaning( ).c_str( ) );
    wiznet( WIZ_LANGUAGE, 0, 0, "%^C1 узнает смысл слова '%s' (%s).", 
            ch, word.toStr( ), word.effect.getValue( ).c_str( ) );

    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" );
    attrHints->addWord( word, true );
}

void Language::doForget( PCharacter *ch, const DLString &arg ) const
{
    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).findAttr<XMLAttributeLanguageHints>( "languageHints" );
    XMLAttributeLanguage::Pointer attr = ch->getAttributes( ).findAttr<XMLAttributeLanguage>( "language" );

    if (arg.empty()) {
        ch->pecho("Что именно ты хочешь забыть?");
        return;
    }

    if (!attr && !attrHints) {
        ch->pecho("Ты не знаешь никаких слов.");
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

        ch->pecho( "Все приснившиеся тебе слова ускользают из твоей памяти." );
        return;
    }

    // Find and forget a word among the dreams.    
    Word word;
    if (attr && attr->findWord(word, arg)) {
        attr->removeWord(word, ch);
        ch->pecho("Ты забываешь слово %s.", arg.c_str());
        return;
    }

    // Find and forget a word among the rewards or overheard ones.
    if (attrHints && attrHints->hasWord(arg)) {
        attrHints->removeWord(arg);
        ch->pecho("Ты забываешь слово %s.", arg.c_str());
        return;
    }

    ch->pecho("Ты не знаешь такого слова.");
}

void Language::doRemember( PCharacter *ch, const DLString &arg ) const
{
    XMLAttributeLanguageHints::Pointer attrHints = ch->getAttributes( ).getAttr<XMLAttributeLanguageHints>( "languageHints" );

    if (arg.empty()) {
        ch->pecho("Что именно ты хочешь запомнить?");
        return;
    }
    
    if (attrHints->hasWord(arg)) {
        ch->pecho("Ты и так помнишь это слово.");
        return;
    }
    
    Word word;
    if (!locateWord( word, ch, arg )) {
        ch->pecho("Это слово не существует.");
        return;
    }

    attrHints->addWord(word, false); 
    ch->pecho("Ты запоминаешь слово %s.", word.dictum.c_str());
}


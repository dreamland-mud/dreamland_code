/* $Id: social.cpp,v 1.1.2.2.6.11 2009/11/04 03:24:33 rufina Exp $
 * 
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */

#include "social.h"
#include "socialmanager.h"

#include "logstream.h"
#include "grammar_entities_impl.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "object.h"
#include "core/behavior/behavior_utils.h"
#include "room.h"
#include "areaquestutils.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "dreamland.h"
#include "loadsave.h"
#include "merc.h"
#include "act.h"
#include "interp.h"

#include "def.h"
#include "l10n.h"

using namespace Grammar;

DLString act_to_fmt(const char *s);

const DLString SocialHelp::TYPE = "SocialHelp";

SocialHelp::SocialHelp()
{
}

SocialHelp::~SocialHelp()
{
}

void SocialHelp::setSocial(Social::Pointer social)
{
    this->social = social;
    addAutoKeyword(social->getName());
    addAutoKeyword(social->getRussianName());
    if (!social->getUaName().empty())
        addAutoKeyword(social->getUaName());
    labels.addTransient("social");
    helpManager->registrate( Pointer( this ) );
}

void SocialHelp::unsetSocial()
{
    helpManager->unregistrate( Pointer( this ) );
    social.clear( );
}

void SocialHelp::save() const
{
    if (social)
        social->save();
}

const DLString &Social::getNameFor( lang_t lang ) const
{
    if (lang == LANG_UA && !uaName.getValue().empty())
        return uaName.getValue();
    if (lang != LANG_EN && !rusName.getValue().empty())
        return rusName.getValue();
    return getName();
}

/**
 * Build a per-recipient message out of a social's stored translations. The
 * three-language MultiMessage ctor skips the catalog entirely -- socials keep
 * their translations next to the Russian source, in their own XML.
 */
static MultiMessage social_msg( const XMLMultiString &msg )
{
    return MultiMessage( msg.get(EN), msg.get(RU), msg.get(UA) );
}

MultiMessage Social::getNoargOtherMsg( ) const { return social_msg( msgOthersNoArgument ); }
MultiMessage Social::getNoargMeMsg( ) const { return social_msg( msgCharNoArgument ); }
MultiMessage Social::getAutoOtherMsg( ) const { return social_msg( msgOthersAuto ); }
MultiMessage Social::getAutoMeMsg( ) const { return social_msg( msgCharAuto ); }
MultiMessage Social::getArgOtherMsg( ) const { return social_msg( msgOthersFound ); }
MultiMessage Social::getArgMeMsg( ) const { return social_msg( msgCharFound ); }
MultiMessage Social::getArgVictimMsg( ) const { return social_msg( msgVictimFound ); }
MultiMessage Social::getErrorMsgMsg( ) const { return social_msg( msgCharNotFound ); }
MultiMessage Social::getArgOther2Msg( ) const { return social_msg( msgOthersFound2 ); }
MultiMessage Social::getArgMe2Msg( ) const { return social_msg( msgCharFound2 ); }
MultiMessage Social::getArgVictim2Msg( ) const { return social_msg( msgVictimFound2 ); }
MultiMessage Social::getObjVictimMsg( ) const { return social_msg( msgVictimObj ); }
MultiMessage Social::getObjCharMsg( ) const { return social_msg( msgCharVictimObj ); }
MultiMessage Social::getObjOthersMsg( ) const { return social_msg( msgOthersVictimObj ); }
MultiMessage Social::getObjNoVictimSelfMsg( ) const { return social_msg( msgCharObj ); }
MultiMessage Social::getObjNoVictimOthersMsg( ) const { return social_msg( msgOthersObj ); }

DLString SocialHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (social)
            buf << social->getRussianName().upperFirstCharacter();
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    // Default title if not set explicitly.
    if (title.get(RU).empty() && social)
        return "Социал {c" + social->getRussianName() + "{x, {c" + social->getName() + "{x";
        
    return HelpArticle::getTitle(label);
}

/**
 * Same title, composed in the viewer's language. See AreaHelp for the why:
 * a composed title used to be Russian for every viewer, "toc" included.
 */
DLString SocialHelp::getTitle(const DLString &label, lang_t lang) const
{
    if (!social || !title.get(RU).empty())
        return HelpArticle::getTitle(label, lang);

    if (label == "title")
        return DLString::emptyString;

    // Socials are addressed by name, so the title carries BOTH the viewer's
    // form and the Latin keyword -- either one is what a player would type.
    DLString mine = social->getNameFor(lang);
    DLString latin = social->getName();

    if (label == "toc")
        return mine.upperFirstCharacter();

    if (mine == latin)
        return help_title_fmt(lang, _("Социал {c%1$s{x"), latin);

    return help_title_fmt(lang, _("Социал {c%1$s{x, {c%2$s{x"), mine, latin);
}

// The stand-in item the demo lines are rendered against, declined where the
// language declines.
static InflectedString object_name(lang_t lang)
{
    switch (lang) {
        case LANG_EN: return InflectedString("dagger");
        case LANG_UA: return InflectedString("кинджал||а|у||ом|і");
        default:      return InflectedString("кинжал||а|у||ом|е");
    }
}

// Tranforms player info into structure that 'format' functions would understand:
// russian name with cases and gender.
static InflectedString russian_string(PCMemoryInterface *pc)
{
    MultiGender mg = MultiGender(pc->getSex(), Number::SINGULAR);
    if (pc->getRussianName().getFullForm().empty())
        return InflectedString(pc->getName(), mg);
    else
        return InflectedString(pc->getRussianName().getFullForm(), mg);
}

// Same, for a viewer reading English: the Latin name, which has no cases.
static InflectedString player_string(PCMemoryInterface *pc, lang_t lang)
{
    if (lang == LANG_EN)
        return InflectedString(pc->getName(), MultiGender(pc->getSex(), Number::SINGULAR));

    return russian_string(pc);
}

// Finds random registered player and returns its name+gender.
static InflectedString player_name(lang_t lang)
{
    static InflectedString empty;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();
    int totalFound = 0;
    PCMemoryInterface *result = 0;

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        PCMemoryInterface *pc = i->second;
        const DLString &rname = pc->getRussianName().getFullForm();

        // A declined example only means something in a language that declines;
        // for English any registered name will do.
        if (lang != LANG_EN) {
            // Ignore players w/o configured Russian name.
            if (rname.empty())
                continue;

            // Ignore players whose names look the same in all cases.
            if (rname.find('|') == DLString::npos)
                continue;
        }

        if (number_range(0, totalFound++) == 0)
            result = pc;
    }

    if (!result)
        return empty;

    return player_string(result, lang);
}

// One label column for the demo table, padded to a fixed width in every
// language so the examples stay aligned. Cyrillic is single-byte in KOI8, so
// byte width is character width.
static const int SOCIAL_LABEL_WIDTH = 15;

static DLString social_label(const DLString &label)
{
    DLString padded = label;

    while (padded.size() < (size_t)SOCIAL_LABEL_WIDTH)
        padded += " ";

    return padded;
}


void SocialHelp::getRawText( Character *ch, ostringstream &buf ) const
{
    if (!social)
        return;
    if (ch->is_npc())
        return;

    lang_t lang = viewerLang(ch);
    DLString mine = social->getNameFor(lang);
    DLString latin = social->getName();
    DLString blank = social_label(DLString::emptyString);

    // A social is addressed by name, so the header carries both the viewer's
    // form and the Latin keyword -- unless they are the same word.
    buf << "%PAUSE%";
    if (mine == latin)
        buf << fmt(ch, _("Социал {c%1$s{x: %2$s"),
                   latin.c_str(), social->getShortDescFor(lang).c_str());
    else
        buf << fmt(ch, _("Социал {c%1$s{x, {c%2$s{x: %3$s"),
                   mine.c_str(), latin.c_str(), social->getShortDescFor(lang).c_str());

    buf << endl << endl
        << fmt(ch, _("Вот как этот социал виден тебе и окружающим, когда он применен...")) << endl;

    InflectedString me = player_string(ch->getPC(), lang);
    InflectedString vict1 = player_name(lang);
    InflectedString vict2 = player_name(lang);
    InflectedString obj = object_name(lang);

    // Every example renders in the reader's own language, falling back to the
    // Russian source for a social that has no translation yet.
    const DLString &autoMe = social->msgCharAuto.getForLang(lang);
    const DLString &autoOther = social->msgOthersAuto.getForLang(lang);
    const DLString &noargMe = social->msgCharNoArgument.getForLang(lang);
    const DLString &noargOther = social->msgOthersNoArgument.getForLang(lang);
    const DLString &argMe = social->msgCharFound.getForLang(lang);
    const DLString &argVictim = social->msgVictimFound.getForLang(lang);
    const DLString &argOther = social->msgOthersFound.getForLang(lang);
    const DLString &argMe2 = social->msgCharFound2.getForLang(lang);
    const DLString &argVictim2 = social->msgVictimFound2.getForLang(lang);
    const DLString &argOther2 = social->msgOthersFound2.getForLang(lang);
    const DLString &objSelf = social->msgCharObj.getForLang(lang);
    const DLString &objOthers = social->msgOthersObj.getForLang(lang);
    const DLString &objCharVict = social->msgCharVictimObj.getForLang(lang);
    const DLString &objVict = social->msgVictimObj.getForLang(lang);
    const DLString &objOthersVict = social->msgOthersVictimObj.getForLang(lang);

    if (!autoMe.empty()) {
        buf << endl
            << social_label(fmt(ch, _("На себя:")))
            << fmt(ch, act_to_fmt(autoMe.c_str()).c_str(), &me) << endl;
        if (!autoOther.empty())
            buf << blank << fmt(ch, act_to_fmt(autoOther.c_str()).c_str(), &me) << endl;
    }

    if (!noargMe.empty()) {
        buf << endl
            << social_label(fmt(ch, _("Без параметра:")))
            << fmt(ch, act_to_fmt(noargMe.c_str()).c_str(), &me) << endl;
        if (!noargOther.empty())
            buf << blank << fmt(ch, act_to_fmt(noargOther.c_str()).c_str(), &me) << endl;
    }

    if (!argVictim.empty()) {
        buf << endl
            << social_label(fmt(ch, _("На кого-то:")))
            << fmt(ch, act_to_fmt(argMe.c_str()).c_str(), &me, 0, &vict1) << endl
            << blank << fmt(ch, act_to_fmt(argVictim.c_str()).c_str(), &me, 0, &vict1) << endl
            << blank << fmt(ch, act_to_fmt(argOther.c_str()).c_str(), &me, 0, &vict1) << endl;

        if (!argVictim2.empty()) {
            buf << endl
                << social_label(fmt(ch, _("На двоих:")))
                << fmt(ch, argMe2.c_str(), &me, &vict1, &vict2) << endl
                << blank << fmt(ch, argVictim2.c_str(), &me, &vict1, &vict2) << endl
                << blank << fmt(ch, argOther2.c_str(), &me, &vict1, &vict2) << endl;
        }
    }

    if (!objSelf.empty()) {
        buf << endl
            << social_label(fmt(ch, _("На предмет:")))
            << fmt(ch, objSelf.c_str(), &me, &obj) << endl
            << blank << fmt(ch, objOthers.c_str(), &me, &obj) << endl;
    }

    if (!objVict.empty()) {
        buf << endl
            << social_label(fmt(ch, _("На предмет")))
            << fmt(ch, objCharVict.c_str(), &me, &vict1, &obj) << endl
            << social_label(fmt(ch, _("и персонажа:")))
            << fmt(ch, objVict.c_str(), &me, &vict1, &obj) << endl
            << blank << fmt(ch, objOthersVict.c_str(), &me, &vict1, &obj) << endl;
    }
}

Social::Social( ) : position( POS_RESTING, &position_table )
{
}

Social::~Social( )
{
}

void Social::loaded()
{
    if (!help) {
        LogStream::sendWarning() << "Social " << getName() << " with empty help article." << endl;
        return;
    }
   
    help->setSocial(Pointer(this)); 
}

void Social::unloaded()
{
    if (help) {
        help->unsetSocial();
    }
}

bool Social::matches( const DLString& argument ) const
{
    if (argument.empty( )) 
        return false;
    
    if (SocialBase::matches( argument ))
        return true;

    // SocialBase only knows the Latin keyword and the Russian name; the
    // Ukrainian one is stored here, and a social nobody can type is no social.
    if (!uaName.getValue( ).empty( ) && argument.strPrefix( uaName.getValue( ) ))
        return true;

    for (XMLStringList::const_iterator a = aliases.begin( ); a != aliases.end( ); a++)
        if (argument.strPrefix( *a ))
            return true;
    
    return false;
}

static bool mprog_social( Character *ch, Character *actor, Character *victim, const char *social )
{
    aquest_trigger(ch, actor, "Social", "CCCs", ch, actor, victim, social);
    FENIA_CALL( ch, "Social", "CCs", actor, victim, social );
    FENIA_NDX_CALL( ch->getNPC( ), "Social", "CCCs", ch, actor, victim, social );
    BEHAVIOR_CALL( ch->getNPC( ), social, actor, victim, social );
    return false;
}

static bool oprog_social( Object *obj, Character *actor, Character *victim, const char *social )
{
    FENIA_CALL( obj, "Social", "CCs", actor, victim, social );
    FENIA_NDX_CALL( obj, "Social", "OCCs", obj, actor, victim, social );
    return false;
}

bool Social::mprog( Character *ch, Character *victim )
{
    bool rc = false;

    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room) {
        if (mprog_social( rch, ch, victim, getName( ).c_str( ) ))
            rc = true;

        for (Object *obj = rch->carrying; obj; obj = obj->next_content)
            if (oprog_social( obj, ch, victim, getName( ).c_str( ) ))
                rc = true;
    }

    return rc;
}

static bool rprog_social( Room *room, Character *actor, Character *victim, const char *social, const char *arg)
{
    FENIA_CALL( room, "Social", "CCss", actor, victim, social, arg );
    return false;
}

bool Social::reaction( Character *ch, Character *victim, const DLString &arg )
{
    if (rprog_social( ch->in_room, ch, victim, getName( ).c_str( ), arg.c_str( ) ))
        return true;

    if (mprog( ch, victim ))
        return true;
    
    if (!victim || victim == ch)
        return false;

    if (ch->is_npc( ) || !victim->is_npc( ) || victim->desc)
        return false;
        
    if (IS_CHARMED(victim) || !IS_AWAKE(victim))
        return false;
    
    switch (number_bits( 4 )) {
    case 0:
    case 1: case 2: case 3: case 4:
    case 5: case 6: case 7: case 8:
        oldact( getArgOtherMsg( ), victim, 0, ch, TO_NOTVICT );
        oldact_p( getArgMeMsg( ), victim, 0, ch, TO_CHAR, getPosition( ) );
        oldact( getArgVictimMsg( ), victim, 0, ch, TO_VICT );
        break;

    case 9: case 10: case 11: case 12:
        oldact(_("$c1 шлепает $C4."),  victim, 0, ch, TO_NOTVICT );
        oldact_p(_("Ты шлепаешь $C4."),  victim, 0, ch, TO_CHAR, getPosition( ) );
        oldact(_("$c1 шлепает тебя."), victim, 0, ch, TO_VICT );
        break;
    case 13: 
        interpret_fmt( victim, "sigh %s", ch->getNameC() );
        break;
    case 14:
        interpret_fmt( victim, "shrug %s", ch->getNameC() );
        break;
    case 15: 
        interpret_fmt( victim, "eyebrow %s", ch->getNameC() );
        break;
    }

    return false;
}


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
#include "behavior_utils.h"
#include "room.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "dreamland.h"
#include "loadsave.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "mercdb.h"
#include "def.h"

using namespace Grammar;

DLString act_to_fmt(const char *s);

const DLString SocialHelp::TYPE = "SocialHelp";

SocialHelp::SocialHelp(Social::Pointer social)
{
    this->social = social;
}

SocialHelp::~SocialHelp()
{
}

DLString SocialHelp::getTitle(const DLString &label) const
{
    if (social)
        return social->getName() + ", " + social->getRussianName();
    return HelpArticle::getTitle(label);
}

static const RussianString & object_name()
{
    static RussianString obj("кинжал||а|у||ом|е");
    return obj;
}

// Tranforms player info into structure that 'format' functions would understand:
// russian name with cases and gender.
static RussianString russian_string(PCMemoryInterface *pc)
{
    MultiGender mg = MultiGender(pc->getSex(), Number::SINGULAR);
    return RussianString(pc->getRussianName().getFullForm(), mg);
}

// Finds random registered player and returns its name+gender.
static RussianString player_name()
{
    static RussianString empty;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();
    int totalFound = 0;
    PCMemoryInterface *result = 0;

    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        PCMemoryInterface *pc = i->second;
        const DLString &rname = pc->getRussianName().getFullForm();
    
        // Ignore players w/o configured Russian name.    
        if (rname.empty())
            continue;

        // Ignore players whose names look the same in all cases.
        if (rname.find('|') == DLString::npos)
            continue;
        
        if (number_range(0, totalFound++) == 0)
            result = pc;
    }

    if (!result)
        return empty;

    return russian_string(result);
}


void SocialHelp::getRawText( Character *ch, ostringstream &buf ) const
{
    if (!social)
        return;
    if (ch->is_npc())
        return;

    buf << "Социал {c" << social->getName() << "{x, {c"
        << social->getRussianName() << "{x: " 
        << social->getShortDesc() << endl << endl
        << "Вот как этот социал виден тебе и окружающим, когда он применен..." << endl;

    RussianString me = russian_string(ch->getPC());
    RussianString vict1 = player_name();
    RussianString vict2 = player_name();
    const RussianString &obj = object_name();

    if (!social->getAutoMe().empty()) {
        buf << endl
            << "На себя:       " << fmt(0, act_to_fmt(social->getAutoMe().c_str()).c_str(), &me) << endl;
        if (!social->getAutoOther().empty())
            buf << "               " << fmt(0, act_to_fmt(social->getAutoOther().c_str()).c_str(), &me) << endl;
    }

    if (!social->getNoargMe().empty()) {
        buf << endl
            << "Без параметра: " << fmt(0, act_to_fmt(social->getNoargMe().c_str()).c_str(), &me) << endl;
        if (!social->getNoargOther().empty())
            buf << "               " << fmt(0, act_to_fmt(social->getNoargOther().c_str()).c_str(), &me) << endl;
    }
    
    if (!social->getArgVictim().empty()) {
        buf << endl
            << "На кого-то:    " <<  fmt(0, act_to_fmt(social->getArgMe().c_str()).c_str(), &me, 0, &vict1) << endl
            << "               " <<  fmt(0, act_to_fmt(social->getArgVictim().c_str()).c_str(), &me, 0, &vict1) << endl
            << "               " <<  fmt(0, act_to_fmt(social->getArgOther().c_str()).c_str(), &me, 0, &vict1) << endl;
        
        if (!social->getArgVictim2().empty()) {
            buf << endl
                << "На двоих:      " <<  fmt(0, social->getArgMe2().c_str(), &me, &vict1, &vict2) << endl
                << "               " <<  fmt(0, social->getArgVictim2().c_str(), &me, &vict1, &vict2) << endl
                << "               " <<  fmt(0, social->getArgOther2().c_str(), &me, &vict1, &vict2) << endl;
        }
    }

    if (!social->getObjNoVictimSelf().empty()) {
        buf << endl
            << "На предмет:    " <<  fmt(0, social->getObjNoVictimSelf().c_str(), &me, &obj) << endl
            << "               " <<  fmt(0, social->getObjNoVictimOthers().c_str(), &me, &obj) << endl;
    }

    if (!social->getObjVictim().empty()) {
        buf << endl
            << "На предмет     " <<  fmt(0, social->getObjChar().c_str(), &me, &vict1, &obj) << endl
            << "и персонажа:   " <<  fmt(0, social->getObjVictim().c_str(), &me, &vict1, &obj) << endl
            << "               " <<  fmt(0, social->getObjOthers().c_str(), &me, &vict1, &obj) << endl;
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
    help = SocialHelp::Pointer(NEW, Pointer(this));
    help->addKeyword(getName());
    help->addKeyword(getRussianName());
    help->addLabel("social");
    
    helpManager->registrate(help);
}

void Social::unloaded()
{
    if (help) {
        helpManager->unregistrate(help);
        help.clear(); 
    }
}

bool Social::matches( const DLString& argument ) const
{
    if (argument.empty( )) 
        return false;
    
    if (SocialBase::matches( argument ))
        return true;
    
    for (XMLStringList::const_iterator a = aliases.begin( ); a != aliases.end( ); a++)
        if (argument.strPrefix( *a ))
            return true;
    
    return false;
}

static bool mprog_social( Character *ch, Character *actor, Character *victim, const char *social )
{
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
        act( getArgOther( ).c_str( ), victim, 0, ch, TO_NOTVICT );
        act_p( getArgMe( ).c_str( ), victim, 0, ch, TO_CHAR, getPosition( ) );
        act( getArgVictim( ).c_str( ), victim, 0, ch, TO_VICT );
        break;

    case 9: case 10: case 11: case 12:
        act( "$c1 шлепает $C4.",  victim, 0, ch, TO_NOTVICT );
        act_p( "Ты шлепаешь $C4.",  victim, 0, ch, TO_CHAR, getPosition( ) );
        act( "$c1 шлепает тебя.", victim, 0, ch, TO_VICT );
        break;
    case 13: 
        interpret_fmt( victim, "sigh %s", ch->getNameP( ) );
        break;
    case 14:
        interpret_fmt( victim, "shrug %s", ch->getNameP( ) );
        break;
    case 15: 
        interpret_fmt( victim, "eyebrow %s", ch->getNameP( ) );
        break;
    }

    return false;
}


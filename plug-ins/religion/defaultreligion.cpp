/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultreligion.h"
#include "religionattribute.h"
#include "logstream.h"

#include "xmltableloaderplugin.h"
#include "pcharacter.h"
#include "race.h"
#include "liquid.h"
#include "object.h"
#include "skill.h"
#include "skillmanager.h"
#include "skillgroup.h"

#include "religionflags.h"
#include "liquidflags.h"
#include "websocketrpc.h"
#include "merc.h"
#include "def.h"

static const DLString LABEL_RELIGION = "religion";

TABLE_LOADER_IMPL(ReligionLoader, "religions", "Religion");

/*-------------------------------------------------------------------
 * ReligionHelp 
 *------------------------------------------------------------------*/
const DLString ReligionHelp::TYPE = "ReligionHelp";

void ReligionHelp::setReligion( DefaultReligion::Pointer religion )
{
    this->religion = religion;
    
    addAutoKeyword( religion->getName( ) );
    addAutoKeyword( religion->getRussianName( ).ruscase( '1' ) );
    labels.addTransient(LABEL_RELIGION);

    helpManager->registrate( Pointer( this ) );
}

void ReligionHelp::unsetReligion( )
{
    helpManager->unregistrate( Pointer( this ) );
    religion.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}

void ReligionHelp::save() const
{
    if (religion)
        religion->save();

}
DLString ReligionHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (religion)
            buf << "Религия '" << religion->getRussianName().ruscase('1')  << "'";
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    // Default title if not set explicitly.
    if (titleAttribute.empty() && religion)
        return "Религия {c" + religion->getRussianName().ruscase('1') + "{x, {c" + religion->getName().upperFirstCharacter() + "{x";

    return HelpArticle::getTitle(label);
}

void ReligionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Религия {C" << religion->getRussianName().ruscase('1') << "{x ({C"
       << religion->getShortDescr() << "{x)";
    
    if (ch && ch->desc) {
        if (religion->available(ch))
            in << ", доступна для тебя.";
        else
            in << ", недоступна тебе.";
    }

    in << " " << "%PAUSE% " << web_edit_button(ch, "reledit", religion->getName()) << "%RESUME%" << endl << endl
       << *this;
}

/*----------------------------------------------------------------------
 * DefaultReligion 
 *---------------------------------------------------------------------*/
DefaultReligion::DefaultReligion( )
                : align( 0, &align_flags ),
                  ethos( 0, &ethos_flags ),
                  races( raceManager ),
                  classes( professionManager ),
                  sex( SEX_MALE, &sex_table ),
                  flags( 0, &religion_flags ),
                  minstat(&stat_table), maxstat(&stat_table),
                  clans(clanManager)

{
}


const DLString & DefaultReligion::getName( ) const
{
    return Religion::getName( );
}

void DefaultReligion::setName( const DLString &name ) 
{
    this->name = name;
}

bool DefaultReligion::isValid( ) const
{
    return true;
}

bool DefaultReligion::available( Character *ch ) const
{
    return reasonWhy(ch).empty();
}

DLString DefaultReligion::reasonWhy(Character *ch) const
{
    static const DLString OK = DLString::emptyString;

    if (flags.isSet(RELIG_SYSTEM))
        return "hidden";

    if (!clans.empty() && !clans.isSet(ch->getClan()))
        return "clan";

    if (!minstat.empty()) 
        for (int i = 0; i < stat_table.size; i++)
            if (ch->getCurrStat(i) < minstat[i])
                return "minstat";
    
    if (!maxstat.empty())
        for (int i = 0; i < stat_table.size; i++)
            if (maxstat[i] > 0 && ch->getCurrStat(i) > maxstat[i])
                return "maxstat";

    if (minage > 0 && !ch->is_npc() && ch->getPC()->age.getYears() < minage)
        return "too_young";
    
    if (maxage > 0 && !ch->is_npc() && ch->getPC()->age.getYears() > maxage)
        return "too_old";

    if (ethos.getValue() > 0 && !ethos.isSetBitNumber(ch->ethos))
        return "ethos";

    if (align.getValue() > 0 && !align.isSetBitNumber(ALIGNMENT(ch)))
        return "align";

    if (races.empty() && classes.empty())
        return OK;
        
    bool myRace = races.isSet(ch->getRace());
    bool myClass = classes.isSet(ch->getProfession());

    // Allow "thief OR kender" restriction if both race and class are set in religion profile.
    if (!races.empty() && !classes.empty()) {
        if (myRace || myClass)
            return OK;
        else if (!myRace)
            return "race";
        else
            return "class";
    }

    // Allow "vampire only" restriction if race is not set.
    if (races.empty())
        return myClass ? OK : "class";
    else
        return myRace ? OK : "race";
}

const DLString &DefaultReligion::getRussianName( ) const
{
    return nameRus;
}

const DLString& DefaultReligion::getNameFor( Character *looker ) const
{
    if (looker && looker->getSex() == SEX_FEMALE && !nameRusFemale.empty())
        return nameRusFemale;
        
    if (looker && looker->getConfig( ).ruskills) 
        return nameRus;

    return shortDescr;
}

void DefaultReligion::loaded( )
{
    religionManager->registrate( Pointer( this ) );
    if (help)
        help->setReligion(Pointer(this));
}

void DefaultReligion::unloaded( )
{
    if (help)
        help->unsetReligion();

    religionManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultReligion::getShortDescr( ) const
{
    return shortDescr.getValue( );
}

const DLString & DefaultReligion::getDescription( ) const
{
    return description.getValue( );
}

int DefaultReligion::getSex() const
{
    return sex;
}


bool DefaultReligion::likesSpell(Skill *skill) const
{
    if (likes.skills.isSet(skill))
        return true;
    if (likes.skillGroups.isSetAny(skill->getGroups()))
        return true;
    return false;
}

bool DefaultReligion::likesDrink(const Liquid *liq) const
{
    if (likes.liquids.isSet(liq))    
        return true;
    if (liq->getFlags().isSet(likes.liquidFlags.getValue()))
        return true;
    return false;
}

bool DefaultReligion::likesItem(Object *obj) const
{
    if (likes.items.isSetBitNumber(obj->item_type))
        return true;
    
    return false;
}

bool DefaultReligion::likesBook(Object *obj) const
{
    return likes.books;
}

/**
 * Return true if an item should be ignored totally and not included
 * in any sacrifice cost calculations. Currently supports deities that
 * only prefer stolen items: items that can in theory be stolen need
 * to be marked as such.
 */
bool DefaultReligion::ignoresItem(Object *obj) const
{
    if (!likes.stolen)
        return false;

    switch (obj->item_type) {
    case ITEM_MONEY:
    case ITEM_CORPSE_PC:
    case ITEM_CORPSE_NPC:
        return false;
    }    

    return !likesStolen(obj);
}

bool DefaultReligion::likesStolen(Object *obj) const
{
    if (!likes.stolen)
        return false;

    return obj->getProperty("stolen") != "";
}

GodLikes::GodLikes()
            : skills(skillManager), skillGroups(skillGroupManager),
              items(0, &item_table), liquids(liquidManager),
              liquidFlags(0, &liquid_flags), books(false), stolen(false)
{
}



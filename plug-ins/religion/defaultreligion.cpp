/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultreligion.h"
#include "religionattribute.h"
#include "logstream.h"

#include "pcharacter.h"
#include "race.h"
#include "liquid.h"
#include "object.h"
#include "skill.h"
#include "skillmanager.h"
#include "skillgroup.h"

#include "liquidflags.h"
#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * ReligionHelp 
 *------------------------------------------------------------------*/
const DLString ReligionHelp::TYPE = "ReligionHelp";

void ReligionHelp::setReligion( Religion::Pointer religion )
{
    this->religion = religion;
    
    if (!keyword.empty( ))
        keywords.fromString( keyword.toLower() );

    keywords.insert( religion->getName( ) );
    keywords.insert( religion->getRussianName( ).ruscase( '1' ) );
    fullKeyword = keywords.toString( ).toUpper( );

    helpManager->registrate( Pointer( this ) );
}

void ReligionHelp::unsetReligion( )
{
    helpManager->unregistrate( Pointer( this ) );
    religion.clear( );
    keywords.clear();
    fullKeyword = "";
}

struct CommaSet : public set<string> {
    void print( ostream &buf ) const {
        bool found = false;
        for (const_iterator i = begin( ); i != end( ); i++) {
            if (found)
                buf << ", ";
            buf << *i;
            found = true;
        }
    }
};
inline ostream& operator << ( ostream& ostr, const CommaSet& cset )
{
    cset.print( ostr );
    return ostr;
}

void ReligionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Религия {C" << religion->getRussianName().ruscase('1') << "{x ({C"
       << religion->getShortDescr() << "{x), ";

    if (religion->isAllowed(ch))
        in << "доступна для тебя.";
    else
        in << "недоступна тебе.";
    in << endl << endl
       << *this;
}

/*----------------------------------------------------------------------
 * DefaultReligion 
 *---------------------------------------------------------------------*/
DefaultReligion::DefaultReligion( )
                : align( 0, &align_table ),
                  ethos( 0, &ethos_table ),
                  races( raceManager ),
                  classes( professionManager ),
                  sex( SEX_MALE, &sex_table )

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

bool DefaultReligion::isAllowed( Character *ch ) const
{
    if (!ethos.isSetBitNumber( ch->ethos ))
        return false;

    if (!align.isSetBitNumber( ALIGNMENT(ch) ))
        return false;

    bool raceOK = races.empty() || races.isSet(ch->getRace());
    bool classOK = classes.empty() || classes.isSet(ch->getProfession());

    return raceOK || classOK;
}

const DLString &DefaultReligion::getRussianName( ) const
{
    return nameRus;
}

const DLString& DefaultReligion::getNameFor( Character *looker ) const
{
    if (!looker || !looker->getConfig( )->ruskills) 
        return shortDescr;

    return nameRus;
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
    if (likes.skillGroups.isSet(skill->getGroup()))
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

    return obj->properties.count("stolen") != 0;
}

GodLikes::GodLikes()
            : skills(skillManager), skillGroups(skillGroupManager),
              items(0, &item_table), liquids(liquidManager),
              liquidFlags(0, &liquid_flags), books(false), stolen(false)
{
}



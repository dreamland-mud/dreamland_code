/* $Id: npcharacter.cpp,v 1.1.2.15.6.19 2009/09/17 18:08:56 rufina Exp $
 * 
 * ruffina, 2004
 */

#include "logstream.h"
#include "class.h"
#include "noun.h"
#include "grammar_entities_impl.h"
#include "ru_pronouns.h"

#include "fenia/register-impl.h"

#include "wrapperbase.h"
#include "feniamanager.h"

#include "mobilebehavior.h"
#include "skill.h"
#include "skillmanager.h"
#include "skillreference.h"
#include "spelltarget.h"

#include "object.h"
#include "room.h"
#include "affect.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "race.h"
#include "merc.h"
#include "dreamland.h"
#include "def.h"

GSN(mirror);
GSN(charm_person);
PROF(none);

NPCharacter::NPCharacter( ) :
                reset_room(0),
                behavior( MobileBehavior::NODE_NAME )
{
    init( );
}

NPCharacter::~NPCharacter( )
{
}

/****************************************************************************
 * recycle 
 ****************************************************************************/
void NPCharacter::init( )
{
    Character::init( );

    pIndexData = 0;
    zone = 0;

    description.clear();
    long_descr.clear();
    short_descr.clear();
    cachedNoun.clear( );
        
    group = 0;
    off_flags = 0;
    for (int i = 0; i < 3; i++ ) 
        damage[i] = 0;
    start_pos = POS_STANDING;
    default_pos = POS_STANDING;
    
    profession.assign( prof_none );
    spec_fun.clear( );
    behavior.clear( );
    reset_room = 0;
    switchedFrom = 0;
}

/**************************************************************************
 * gate to pc/npc info 
 **************************************************************************/
PCharacter *NPCharacter::getPC()
{
    return switchedFrom;
}

NPCharacter *NPCharacter::getNPC()
{
    return this;
}

const PCharacter *NPCharacter::getPC() const
{
    return switchedFrom;
}

const NPCharacter *NPCharacter::getNPC() const
{
    return this;
}

bool NPCharacter::is_npc( ) const
{
    return true;
}

/****************************************************************************
 * additional set/get methods 
 ****************************************************************************/

short NPCharacter::getModifyLevel( ) const 
{
    return level.getValue( );
}

/************************************************************************
 * set-get methods for string fields
 ************************************************************************/
void NPCharacter::setDescription( const DLString& d, lang_t lang )
{
    description[lang] = d;
}

const char * NPCharacter::getDescription( lang_t lang ) const
{
    const DLString &mydesc = description.get(lang);
    return mydesc.empty() ? pIndexData->description.get(lang).c_str() : mydesc.c_str();
}

const XMLMultiString & NPCharacter::getDescription( ) const
{
    return description;
}

void NPCharacter::setDescription( const XMLMultiString &description )
{
    this->description = description;
} 

const char * NPCharacter::getShortDescr( lang_t lang ) const
{
    const DLString &myshort = short_descr.get(lang);
    return myshort.empty() ? pIndexData->short_descr.get(lang).c_str() : myshort.c_str();
}

const char * NPCharacter::getLongDescr( lang_t lang ) const
{
    const DLString &mylong = long_descr.get(lang);
    return mylong.empty() ? pIndexData->long_descr.get(lang).c_str() : mylong.c_str();
}

const char * NPCharacter::getRealShortDescr( lang_t lang ) const
{
    return short_descr.get(lang).c_str();
}

const char * NPCharacter::getRealLongDescr( lang_t lang ) const
{
    return long_descr.get(lang).c_str();
}

const char * NPCharacter::getRealDescription( lang_t lang ) const
{
    return description.get(lang).c_str();;
}

void NPCharacter::setShortDescr( const char *d, lang_t lang )
{
    short_descr[lang] = d;
    updateCachedNoun( );
}

void NPCharacter::setLongDescr( const char *d, lang_t lang )
{
    long_descr[lang] = d;
}

void NPCharacter::setShortDescr( const DLString& d, lang_t lang )
{
    setShortDescr( d.c_str( ), lang );
}

void NPCharacter::setLongDescr( const DLString& d, lang_t lang )
{
    setLongDescr( d.c_str( ), lang );
}

/*****************************************************************************
 * name and sex formatting
 *****************************************************************************/
using namespace Grammar;

Noun::Pointer NPCharacter::toNoun( const DLObject *forWhom, int flags ) const
{
    const Character *wch = dynamic_cast<const Character *>(forWhom);
    
    if (IS_SET(flags, FMT_INVIS)) {
        if (wch && !wch->can_see( this ))
            return somebody;
    }
    
    return cachedNoun;
}

void NPCharacter::updateCachedNoun( )
{
    DLString fullForm = getShortDescr(LANG_RU);

    if (!cachedNoun) {
        cachedNoun = RussianString::Pointer( 
                        NEW, 
                        fullForm, 
                        MultiGender( getSex( ), pIndexData->gram_number) );
    }
    else {
        cachedNoun->setFullForm(fullForm);
        cachedNoun->setGender( MultiGender( getSex( ), pIndexData->gram_number ) );
    }
}

DLString NPCharacter::getNameP( char gram_case ) const
{   
    // Special case when we want to see all names at once, to search among them.
    if (gram_case == '7') {
        DLString fullForm;

        // Summoned creatures may contain master's name in their short descr, hence
        // their name is handled differently, with the last part of the short descr removed.
        if (behavior && behavior->hasSpecialName()) {
            StringList names(getShortDescr(LANG_RU));
            names.pop_back();
            fullForm = russian_case_all_forms(names.toString());
        } else {
            fullForm = toNoun()->decline(gram_case).colourStrip();
        }
 
        return fullForm + " " + getName();
    }

    return toNoun()->decline(gram_case);
}

/****************************************************************************
 * trust and immortality 
 ****************************************************************************/
int NPCharacter::get_trust( ) const
{
    if ( switchedFrom )
        return switchedFrom->get_trust();
    
    return std::min( (int)getRealLevel( ), LEVEL_HERO );
}

bool NPCharacter::isCoder( ) const
{
    return false;
}

bool NPCharacter::is_immortal( ) const
{
    return false;
}

/****************************************************************************
 * character stats 
 ****************************************************************************/
int NPCharacter::getCurrStat( int stat ) 
{
    int value;

    // Mob's perm_stat is set only once when the mob is created, all other modifiers are applied on-the-fly.
    // TODO: we might allow mob's stat to go above the max threshold in some cases, but existing battle formulae
    // need to be reviewed first.
    value = perm_stat[stat];
    value += getRace()->getStats()[stat];
    value += mod_stat[stat];
    value += getProfession( )->getStat( stat, this ); 
    return URANGE( MIN_STAT, value, MAX_STAT );
}

/****************************************************************************
 * misc 
 ****************************************************************************/
bool NPCharacter::is_vampire( ) const
{
    return false;
}
bool NPCharacter::is_mirror( ) const
{
    return isAffected( gsn_mirror );
}

/****************************************************************************
 * switched player configuration 
 ****************************************************************************/
PlayerConfig NPCharacter::getConfig( ) const
{
    if (switchedFrom)
        return switchedFrom->getConfig( );
    else
        return PlayerConfig();
}




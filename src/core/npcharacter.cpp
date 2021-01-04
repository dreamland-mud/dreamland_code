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

#include "dreamland.h"
#include "mercdb.h"
#include "def.h"

GSN(mirror);
GSN(charm_person);
PROF(none);

NPCharacter::NPCharacter( ) :
                reset_room(0),
                description( 0 ),
                short_descr( 0 ),
                long_descr( 0 ),
                behavior( MobileBehavior::NODE_NAME )
{
    init( );
}

NPCharacter::~NPCharacter( )
{
    mobile_count--;
}

/****************************************************************************
 * recycle 
 ****************************************************************************/
void NPCharacter::init( )
{
    Character::init( );

    pIndexData = 0;
    zone = 0;

    free_string( description );
    description = 0;
    free_string(short_descr);
    short_descr = 0;
    free_string(long_descr);
    long_descr = 0;
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
const GlobalBitvector & NPCharacter::getWearloc( ) 
{
    return race->getWearloc( );
}

short NPCharacter::getModifyLevel( ) const 
{
    return level.getValue( );
}

/************************************************************************
 * set-get methods for string fields
 ************************************************************************/
void NPCharacter::setDescription( const DLString& d )
{
    if (description)
        free_string( description );

    description = str_dup( d.c_str( ) );
}
const char * NPCharacter::getDescription( ) const
{
    return description ? description : pIndexData->description;
}
void NPCharacter::setShortDescr( const char *d )
{
    if (short_descr)
        free_string( short_descr );

    short_descr = str_dup( d );
    updateCachedNoun( );
}
void NPCharacter::setLongDescr( const char *d )
{
    if (long_descr)
        free_string( long_descr );

    long_descr = str_dup( d );
}
void NPCharacter::setShortDescr( const DLString& d )
{
    setShortDescr( d.c_str( ) );
}
void NPCharacter::setLongDescr( const DLString& d )
{
    setLongDescr( d.c_str( ) );
}

void NPCharacter::fmtName( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setName( buf );
}

void NPCharacter::fmtShortDescr( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setShortDescr( buf );
}

void NPCharacter::fmtLongDescr( const char *fmt, ... )
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start( ap, fmt );
    vsprintf( buf, fmt, ap );
    va_end( ap );

    setLongDescr( buf );
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
    if (!cachedNoun) {
        cachedNoun = RussianString::Pointer( 
                        NEW, 
                        getShortDescr( ), 
                        MultiGender( getSex( ), pIndexData->gram_number) );
    }
    else {
        cachedNoun->setFullForm( getShortDescr( ) );
        cachedNoun->setGender( MultiGender( getSex( ), pIndexData->gram_number ) );
    }
}

DLString NPCharacter::getNameP( char gram_case ) const
{   
    DLString shortname = toNoun( )->decline( gram_case );

    // Special case when we want to see all names at once, to search among them.
    if (gram_case == '7')
        return shortname.colourStrip() + " " + getName();

    return shortname;
}

/****************************************************************************
 * npc skills 
 ****************************************************************************/

int NPCharacter::applyCurse( int def )
{
    return def;
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

    value = perm_stat[stat];
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


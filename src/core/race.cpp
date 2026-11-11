/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "enumeration.h"
#include "globalbitvector.h"
#include "race.h"
#include "body.h"
#include "pcrace.h"
#include "skillreference.h"
#include "merc.h"
#include "def.h"

GSN(faerie_fire);
GSN(infravision);
GSN(fly);
GSN(curse);
GSN(regeneration);
GSN(sneak);
GSN(hide);
GSN(pass_door);
GSN(protection_good);
GSN(slow);
GSN(swim);

/** Trait affects whose handler sets an affect bit, and that bit. */
static const struct {
    SkillReference *skill;
    bitstring_t bit;
} affect_trait_table[] = {
    { &gsn_faerie_fire,     AFF_FAERIE_FIRE },
    { &gsn_infravision,     AFF_INFRARED },
    { &gsn_fly,             AFF_FLYING },
    { &gsn_curse,           AFF_CURSE },
    { &gsn_regeneration,    AFF_REGENERATION },
    { &gsn_sneak,           AFF_SNEAK },
    { &gsn_hide,            AFF_HIDE },
    { &gsn_pass_door,       AFF_PASS_DOOR },
    { &gsn_protection_good, AFF_PROTECT_GOOD },
    { &gsn_slow,            AFF_SLOW },
    { &gsn_swim,            AFF_SWIM },
};

bitstring_t affect_trait_bits( const GlobalBitvector &affects )
{
    bitstring_t bits = 0;

    for (auto &row: affect_trait_table) {
        int sn = *row.skill;
        if (affects.isSet( sn ))
            bits |= row.bit;
    }

    return bits;
}

/*-------------------------------------------------------------------
 * Race
 *------------------------------------------------------------------*/
Race::Race( )
{
}

Race::Race( const DLString &n ) : name( n )
{
}

Race::~Race( )
{
}

const DLString& Race::getName( ) const
{
    return name;
}

bool Race::isValid( ) const
{
    return false;
}

PCRace emptyPCRace( "none" );

PCRace * Race::getPC( ) 
{
    return &emptyPCRace;
}

bool Race::isPC( ) const
{
    return false;
}

const Flags & Race::getDet( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getAct( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getAff( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getOff( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getImm( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getRes( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getVuln( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getForm( ) const
{
    return Flags::emptyFlags;
}
const Flags & Race::getParts( ) const
{
    return Flags::emptyFlags;
}
const EnumerationArray & Race::getStats( ) const
{
    return EnumerationArray::emptyArray;
}
const GlobalBitvector & Race::getWearloc( ) const
{
    return GlobalBitvector::emptyBitvector;
}
const GlobalBitvector & Race::getAffects( ) const
{
    return GlobalBitvector::emptyBitvector;
}
const Enumeration & Race::getSize( ) const
{
    return Enumeration::emptyEnumeration;
}
Flags Race::getAttitude( const Race & ) const
{
    return Flags::emptyFlags;
}

const DLString & Race::getMaleName( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getNeuterName( ) const
{
    return getMaleName( );
}
const DLString & Race::getFemaleName( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getMltName( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getMaleNameUa( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getFemaleNameUa( ) const
{
    return DLString::emptyString;
}
const DLString & Race::getMltNameUa( ) const
{
    return DLString::emptyString;
}

DLString Race::getNameFor( Character *, Character * ) const
{
    return getName( );
}

/*-------------------------------------------------------------------
 * RaceManager
 *------------------------------------------------------------------*/
RaceManager* raceManager = 0;

RaceManager::RaceManager( ) 
{
    checkDuplicate( raceManager );
    raceManager = this;
    setRegistryName("race");    
    saveRegistryName();
}

RaceManager::~RaceManager( )
{
    eraseRegistryName();
    raceManager = 0;
}

GlobalRegistryElement::Pointer RaceManager::getDumbElement( const DLString &name ) const
{
    return Race::Pointer( NEW, name );
}

const PCRace * RaceManager::findUnstrictPC( const DLString &name )
{
    Race *race;
    
    if (name.empty( ))
        return NULL;
        
    if (( race = findExisting( name ) )
        && race->isValid( ) 
        && race->isPC( ))
    {
        return race->getPC( );
    }
    
    for (int i = 0; i < size( ); i++) {
        race = find( i );
        
        if (race->isValid( ) 
            && race->isPC( )
            && name.strPrefix( race->getName( ) ))
        {
            return race->getPC( );
        }
    }

    return NULL;
}

GLOBALREF_IMPL(Race, ' ')
XMLGLOBALREF_IMPL(Race)

void Race::getBodyInput( Body::Input &, bool ) const
{
}

std::vector<std::vector<int> > Race::getAttacks( ) const
{
    return std::vector<std::vector<int> >( );
}

int Race::getDamType( ) const
{
    return 0;
}

const DLString & Race::getMaterial( ) const
{
    return DLString::emptyString;
}

double Race::getHpMult( ) const
{
    return 1.0;
}

double Race::getDmgMult( ) const
{
    return 1.0;
}

const DLString & Race::getMoveVerb( ) const
{
    return DLString::emptyString;
}

const DLString & Race::getSpec( ) const
{
    return DLString::emptyString;
}

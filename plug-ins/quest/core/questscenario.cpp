/* $Id$
 *
 * ruffina, 2004
 */
#include "questscenario.h"
#include "questexceptions.h"

#include "core/object.h"
#include "npcharacter.h"
#include "pcharacter.h"

#include "../../anatolia/handler.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

RACE(none);

QuestScenario::~QuestScenario( )
{
}

bool QuestScenario::applicable( PCharacter *, NPCharacter * ) const
{
    return false;
}

int QuestScenario::getPriority() const
{
    return 1;
}

const DLString &
QuestScenariosContainer::getRandomScenario( PCharacter *ch ) const
{
    Scenarios::const_iterator i, result = scenarios.end( );
    int count = 0;
    
    for (i = scenarios.begin( ); i != scenarios.end( ); i++)
        if (i->second->applicable( ch ))
            if (number_range( 0, count++ ) == 0) 
                result = i;

    if (result == scenarios.end( ))
        throw QuestCannotStartException( );
    
    return result->first;
}

const DLString &
QuestScenariosContainer::getWeightedRandomScenario( PCharacter *ch ) const
{
    int summ = 0;
    map<DLString, QuestScenario::Pointer> applicable;
    map<DLString, QuestScenario::Pointer>::const_iterator a;
 
    for (Scenarios::const_iterator i = scenarios.begin( ); i != scenarios.end( ); i++) {
        if (i->second->applicable( ch )) {
            summ += i->second->getPriority( );
            applicable[i->first] = static_cast<const QuestScenario *>(i->second.getPointer());
        } 
    }

    int dice = number_range( 0, summ - 1 );
    int currentSum = 0;
    for (a = applicable.begin(); a != applicable.end(); a++) {
        currentSum += a->second->getPriority( );
        if (currentSum > dice) 
            return a->first;
    }

    throw QuestCannotStartException( );
}

QuestScenario::Pointer
QuestScenariosContainer::getScenario( const DLString &name ) const
{
    Scenarios::const_iterator i = scenarios.find( name );
    
    if (i == scenarios.end( ))
        throw QuestRuntimeException( "wrong scenario name: " + name );
        
    return i->second;
}

QuestItemAppearence::QuestItemAppearence( )
                        : wear( 0, &wear_flags ),
                          extra( 0, &extra_flags )
{
}

void QuestItemAppearence::dress( Object *obj ) const
{
    if (!name.empty( ))
        obj->setName( (name + " " + obj->pIndexData->name).c_str( ) );
        
    if (!shortDesc.empty( ))
        obj->setShortDescr( shortDesc.c_str( ) );
        
    if (!desc.empty( ))
        obj->setDescription( desc.c_str( ) );

    if (!extraDesc.empty( ))
        obj->addExtraDescr( obj->getName( ), extraDesc );

    SET_BIT( obj->wear_flags, wear.getValue( ) );
    SET_BIT( obj->extra_flags, extra.getValue( ) );
}

QuestMobileAppearence::QuestMobileAppearence( )
                           : sex( SEX_MALE, &sex_table ),
                             align( N_ALIGN_NULL, &align_table )
{
    race.assign( race_none );
}

void QuestMobileAppearence::dress( NPCharacter *mob ) const
{
    mob->setName( name + " " + mob->pIndexData->player_name );
    mob->setShortDescr( shortDesc );
    mob->setLongDescr( longDesc + "\r\n" );
    mob->setDescription( desc + "\r\n" );
    mob->setSex( sex.getValue( ) ); 

    if (race.getName() != "none") {
        mob->setRace( race.getName( ) );
        SET_BIT(mob->form, mob->getRace()->getForm());
        SET_BIT(mob->parts, mob->getRace()->getParts());
        SET_BIT(mob->vuln_flags, mob->getRace()->getVuln());
        SET_BIT(mob->detection, mob->getRace()->getDet());
        SET_BIT(mob->affected_by, mob->getRace()->getAff());
        SET_BIT(mob->imm_flags, mob->getRace()->getImm());
        SET_BIT(mob->res_flags, mob->getRace()->getRes());
        mob->wearloc.set(mob->getRace()->getWearloc());
    }
    
    switch (align.getValue( )) {
    case N_ALIGN_GOOD: mob->alignment = 1000; break;
    case N_ALIGN_EVIL: mob->alignment = -1000; break;
    case N_ALIGN_NEUTRAL: mob->alignment = 0; break;
    }

}


int VnumList::randomVnum( )
{
    if (size( ) == 0)
        return -1;

    return at( number_range( 0, size( ) - 1 ) );
}

Object * VnumList::randomItem( )
{
    int vnum;
    OBJ_INDEX_DATA *pObjIndex;

    if (( vnum = randomVnum( ) ) > 0)
        if (( pObjIndex = get_obj_index( vnum ) ))
            return create_object( pObjIndex, 0 );
    
    return NULL;
}

bool NameList::hasName( NPCharacter *mob )
{
    DLString arg;
    DLString names = mob->pIndexData->player_name;

    while (!( arg = names.getOneArgument( ) ).empty( ))
        if (hasElement( arg ))
            return true;
        
    return false;
}


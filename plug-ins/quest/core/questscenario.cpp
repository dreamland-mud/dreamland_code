/* $Id$
 *
 * ruffina, 2004
 */
#include "questscenario.h"
#include "questexceptions.h"

#include "grammar_entities_impl.h"
#include "core/object.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "string_utils.h"
#include "loadsave.h"

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
    if (!gender.empty())
        obj->gram_gender.fromString(gender.c_str());

    if (!name.empty( ))
        obj->setKeyword(name + " " + String::toString(obj->pIndexData->keyword));
        
    if (!shortDesc.empty( ))
        obj->setShortDescr( shortDesc, LANG_DEFAULT);
        
    if (!desc.empty( ))
        obj->setDescription( desc, LANG_DEFAULT );

    if (!extraDesc.empty( ))
        obj->addProperDescription()->description[LANG_DEFAULT] = extraDesc;

    if (!material.empty())
        obj->setMaterial(material);

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
    mob->setKeyword( name + " " + String::toString(mob->pIndexData->keyword));
    mob->setShortDescr( shortDesc, LANG_RU );
    mob->setLongDescr( longDesc + "\r\n", LANG_RU );
    mob->setDescription( desc + "\r\n", LANG_RU );
    mob->setSex( sex.getValue( ) ); 

    if (race.getName() != "none") {
        mob->setRace( race.getName( ) );
        mob->size = mob->getRace()->getSize();
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
    StringList names = String::getAllForms(mob->pIndexData->keyword);

    for (auto &name: names)
        if (hasElement( name ))
            return true;
        
    return false;
}


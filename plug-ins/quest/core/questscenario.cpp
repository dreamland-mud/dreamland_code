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

StringList QuestScenariosContainer::getScenarioNames( ) const
{
    StringList names;

    for (Scenarios::const_iterator i = scenarios.begin( ); i != scenarios.end( ); i++)
        names.push_back( i->first );

    return names;
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

    // Write every language slot. A slot with no translation of its own gets the
    // Russian text (getForLang), never nothing: an empty instance slot falls
    // through to the *prototype* (String::firstNonEmpty), so leaving it blank
    // would show an EN/UA reader the undressed generic item instead of the
    // quest one. Same policy XMLItemRestring::dress applies.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        // Keyword is PREPENDED to the prototype's own list, never replaces it,
        // so no token a player could already type is lost.
        if (!name.emptyValues( ))
            obj->setKeyword( name.getForLang(lang) + " " + obj->pIndexData->keyword.get(lang), lang );

        if (!shortDesc.emptyValues( ))
            obj->setShortDescr( shortDesc.getForLang(lang), lang );

        if (!desc.emptyValues( ))
            obj->setDescription( desc.getForLang(lang), lang );
    }

    // Only after the keyword loop. addProperDescription() stamps ed->keyword
    // from getKeyword() at call time (object.cpp:234), so calling it earlier
    // files the extra description under the PROTOTYPE's keywords and `look
    // <quest name>` misses it. class_thief.cpp:712 spells out the same order.
    if (!extraDesc.emptyValues( )) {
        ExtraDescription *ed = obj->addProperDescription();

        for (int l = LANG_MIN; l < LANG_MAX; l++)
            ed->description[(lang_t)l] = extraDesc.getForLang((lang_t)l);
    }

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
    // Per-language, for the reasons spelled out in QuestItemAppearence::dress.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        if (!name.emptyValues( ))
            mob->setKeyword( name.getForLang(lang) + " " + mob->pIndexData->keyword.get(lang), lang );

        if (!shortDesc.emptyValues( ))
            mob->setShortDescr( shortDesc.getForLang(lang), lang );

        if (!longDesc.emptyValues( ))
            mob->setLongDescr( longDesc.getForLang(lang), lang );

        if (!desc.emptyValues( ))
            mob->setDescription( desc.getForLang(lang), lang );
    }

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


void QuestMessage::fromXML( const XMLNode::Pointer &node )
{
    XMLContainer::fromXML( node );

    if (!text.emptyValues( ))
        return;

    // Legacy shape: <node>текст</node>, no <text> child at all. Read the body
    // into Russian so an untouched data file keeps working and a rollback to a
    // binary that expects the old shape cannot leave the line blank.
    XMLNode::Pointer body = node->getFirstNode( );

    if (body)
        text[RU] = body->getCData( );
}

MultiMessage questMessage( const XMLMultiString &s )
{
    return MultiMessage( s.getForLang( EN ), s.getForLang( RU ), s.getForLang( UA ) );
}

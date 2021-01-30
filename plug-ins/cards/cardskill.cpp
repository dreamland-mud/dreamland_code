/* $Id: cardskill.cpp,v 1.1.2.8.6.4 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2005
 */
#include "cardskill.h"
#include "cardskillloader.h"                                                    
#include "mobiles.h"
#include "xmlattributecards.h"

#include "skillmanager.h"                                                       
#include "skillgroup.h"                                                       
#include "skill_utils.h"
#include "behavior_utils.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

const DLString CardSkill::CATEGORY = "Умения Колоды";
GROUP(card_pack);
static GlobalBitvector cardGroups(skillGroupManager, group_card_pack);

CardSkill::CardSkill( )
{
}

GlobalBitvector & CardSkill::getGroups( ) 
{
    return cardGroups;
}

bool CardSkill::visible( CharacterMemoryInterface * ch ) const
{
    return isCard( ch );
}

bool CardSkill::available( Character * ch ) const
{
    return findCardLevel( ch ) >= cardLevel.getValue( );
}

bool CardSkill::usable( Character * ch, bool message = false ) const 
{
    return available( ch );
}

int CardSkill::getLevel( Character *ch ) const
{
    return 1;
}

int CardSkill::getLearned( Character *ch ) const
{
    if (!usable( ch, false ))
        return 0;

    return ch->getPC( )->getSkillData( getIndex( ) ).learned;
}

bool CardSkill::canPractice( PCharacter * ch, std::ostream & ) const
{
    return available( ch );
}

bool CardSkill::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose ) 
{
    if (mob && mob->behavior && mob->behavior.getDynamicPointer<CardSellerBehavior>( ))
        return true;

    if (verbose) {    
        if (mob)
            ch->pecho( "%^C1 не разбирается в картах.", mob );
        else
            ch->println( "Поищи кого-то, кто разбирается в картах." );
    }

    return false;
}

void CardSkill::show( PCharacter *ch, std::ostream & buf ) const
{
    buf << print_what(this) << " Колоды "
        << print_names_for(this, ch)
        << print_group_for(this, ch)
        << ".{x" << endl;

    buf << print_wait_and_mana(this, ch);
    
    buf << SKILL_INFO_PAD 
        << "Появляется у карт, начиная с {C" 
        << russian_case( XMLAttributeCards::levelFaces[cardLevel].name, '2' ) 
        << "{x"; 

    if (visible( ch )) {
        int learned = getLearned( ch );
        if (learned > 0)
            buf << ", изучено на {" << skill_learned_colour(this, ch) << learned << "%{x";

        if (!usable( ch ))
            buf << " (сейчас тебе недоступно)";
    }
    
    buf << "." << endl; 
}


/*---------------------------------------------------------------------------
 * 
 *---------------------------------------------------------------------------*/
int CardSkill::findCardLevel( CharacterMemoryInterface *mem ) 
{
    XMLAttributeCards::Pointer attr;
    
    if (!mem->getPCM())
        return -1;
    
    attr = mem->getPCM()->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );

    if (!attr)
        return -1;

    if (attr->isTrump( ))
        return XMLAttributeCards::getMaxLevel( );

    return attr->getLevel( );
}

bool CardSkill::isCard( CharacterMemoryInterface *mem )
{
    return findCardLevel( mem ) >= 0;
}

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

CardSkill::CardSkill( )
{
}

SkillGroupReference & CardSkill::getGroup( ) 
{
    return group_card_pack;
}

bool CardSkill::visible( Character * ch ) const
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

int CardSkill::getWeight( Character * ) const
{
    return 0;
}

bool CardSkill::canForget( PCharacter * ) const
{
    return false;
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

void CardSkill::show( PCharacter *ch, std::ostream & buf ) 
{
    bool rus = ch->getConfig( )->ruskills;

    buf << (spell ? "Заклинание" : "Умение") 
        << " Колоды '{c" << getName( ) << "{x' или '{c" << getRussianName( ) << "{x', "
        << "входит в группу '{hg{c"  
        << (rus ? getGroup( )->getRussianName( ) : getGroup( )->getName( )) 
        << "{x'" << endl << endl;
    
    buf << "Появляется у карт, начиная с {C" 
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

    print_see_also(this, ch, buf);
}


/*---------------------------------------------------------------------------
 * 
 *---------------------------------------------------------------------------*/
int CardSkill::findCardLevel( Character *ch ) 
{
    XMLAttributeCards::Pointer attr;
    
    if (ch->is_npc( ))
        return -1;
    
    attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeCards>( "cards" );

    if (!attr)
        return -1;

    if (attr->isTrump( ))
        return XMLAttributeCards::getMaxLevel( );

    return attr->getLevel( );
}

bool CardSkill::isCard( Character *ch )
{
    return findCardLevel( ch ) >= 0;
}

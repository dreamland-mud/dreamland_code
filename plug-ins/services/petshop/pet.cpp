/* $Id$
 *
 * ruffina, 2004
 */
#include <math.h>
#include <iomanip>

#include "pet.h"

#include "skillreference.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "occupations.h"
#include "interp.h"
#include "act.h"
#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"
#include "skill_utils.h"

GSN(haggle);
RELIG(fili);

/*----------------------------------------------------------------------
 * Pet
 *---------------------------------------------------------------------*/
Pet::Pet( ) 
{
}

void Pet::stopfol( Character *master ) 
{
    lastCharmTime = dreamland->getCurrentTime( );
}

bool Pet::purchase( Character *client, NPCharacter *keeper, const DLString &arguments, int )
{
    NPCharacter *pet;
    
    if (client->is_npc( ) || client->getPC( )->pet) {
        client->pecho( "У тебя уже есть один питомец." );
        return false;
    }

    if (!canAfford( client )) {
        client->pecho( "У тебя не хватает %N2, чтобы заплатить за это.", toCurrency( ).c_str( ) );
        return false;
    }

    if (getLevel( client ) > client->getModifyLevel( )) {
        client->pecho( "У тебя недостаточно опыта, чтобы справиться с этим питомцем." );
        return false;
    }

    deduct( client );
    pet = create( client->getPC( ) ); 
    client->setWaitViolence( 1 );
    
    oldact("В трудную минуту $E поможет тебе!", client, 0, pet, TO_CHAR );
    act("%1$^C1 приобретает %2$C4.", client, pet, 0,TO_ROOM);
    interpret_raw(pet, "report");
    return true;
}

bool Pet::available( Character *client, NPCharacter *keeper ) const
{
    return true;
}

int Pet::getQuantity( ) const
{
    return 1;
}

int Pet::getLevel( Character * ) const
{
    return ch->getRealLevel( );
}

int Pet::toSilver( Character *client ) const
{
    int level = getLevel( client );

    return 10 * level * level;
}

int Pet::haggle( Character *client ) const
{
    int cost = toSilver( client );
    bool bonus = client->getReligion() == god_fili && get_eq_char(client, wear_tattoo) != 0;
    int roll = bonus ? 100 : number_percent( );

    if (bonus || (roll < gsn_haggle->getEffective( client ) + skill_level_bonus(*gsn_haggle, ch))) {
        cost -= cost / 2 * roll / 100;
        client->printf( "Ты торгуешься и цена снижается до %d монет.\r\n", cost );
        gsn_haggle->improve( client, true );
    }
    
    return cost;
}

void Pet::toStream( Character *client, ostringstream &buf ) const
{
    buf << "[" << setw( 3 ) << getLevel( client ) << "] " << setw( 8 )
        << toSilver( client ) << " - " << ch->getNameP( '1' );
}

void Pet::config( PCharacter *client, NPCharacter *pet ) const
{
    SET_BIT( pet->affected_by, AFF_CHARM );
    pet->comm = COMM_NOTELL;
    
    if (IS_SET( pet->act, ACT_NOALIGN ))
        pet->alignment = client->alignment;
    
    pet->setDescription( dlprintf( 
             "%s\r\nТы понимаешь, что %s будет до смерти защищать и следовать за {C%s{x.\n\r", 
             pet->getDescription( ), 
             pet->getNameP( '1' ).c_str( ), client->getNameP( '5' ).c_str( ) ) );
}

NPCharacter * Pet::create( PCharacter *client ) const
{
    NPCharacter *pet;

    pet = create_mobile( ch->pIndexData );
    config( client, pet ); 

    char_to_room( pet, client->in_room );
    
    client->pet = pet;
    pet->add_follower( client );
    pet->leader = client;

    return pet;
}

/*----------------------------------------------------------------------
 * LevelAdaptivePet
 *---------------------------------------------------------------------*/
int LevelAdaptivePet::getLevel( Character *client ) const
{
    return client->getModifyLevel( );
}

void LevelAdaptivePet::config( PCharacter *client, NPCharacter *pet ) const
{
    int level, ave;
    
    Pet::config( client, pet );

    level = getLevel( client );
    pet->setLevel( level );

    if (level <= 13)            pet->hit = level * number_range( 12, 13 );
    else if (level <= 21)   pet->hit = level * number_range( 16, 18 );
    else if (level <= 32)   pet->hit = level * number_range( 25, 30 );
    else if (level <= 40)   pet->hit = level * number_range( 39, 45 );
    else if (level <= 50)   pet->hit = level * number_range( 49, 53 );
    else                    pet->hit = level * number_range( level * 2, level * 2 + 10 );
    pet->max_hit = pet->hit;

    if (level < 20)            pet->mana = number_fuzzy( 100 );
    else if (level < 50)    pet->mana = number_fuzzy( 500 );
    else if (level < 80)    pet->mana = number_fuzzy( 800 );
    else                    pet->mana = number_fuzzy( 1000 );
    pet->max_mana = pet->mana;

    pet->hitroll = number_range( level, level * 2 );

    if (level <= 12)            ave = number_range( 2, 5 );
    else if (level <= 22)   ave = number_range( 6, 9 );
    else if (level <= 32)   ave = number_range( 9, 13 );
    else if (level <= 42)   ave = number_range( 14, 22 );
    else if (level <= 65)   ave = number_range( 23, 25 );
    else if (level <= 75)   ave = number_range( 33, 35 );
    else if (level <= 90)   ave = number_range( 43, 45 );
    else                    ave = number_range( 50, 52 );
    
    pet->damage[DICE_NUMBER] = (int) ::sqrt( 2 * ave );
    pet->damage[DICE_TYPE]   = pet->damage[DICE_NUMBER];
    pet->damroll = number_fuzzy( level / 2 );
    
    pet->saving_throw = -level;
    
    for (int i = 0; i < 4; i++)
        pet->armor[i] = - 5 * number_fuzzy( level );

    if (pet->getRealLevel( ) <= 20) { 
        REMOVE_BIT( pet->act, ACT_CLERIC|ACT_MAGE );
    } 
    else if (IS_SET( pet->act, ACT_CLERIC|ACT_MAGE )) {
        pet->max_mana += level * 10;
        pet->mana = pet->max_mana;
    }
}

/*----------------------------------------------------------------------
 * RideablePet
 *---------------------------------------------------------------------*/
int RideablePet::haggle( Character *client ) const
{
    return MoneyPrice::haggle( client );
}

void RideablePet::config( PCharacter *client, NPCharacter *pet ) const
{
    pet->comm = COMM_NOTELL;
}

NPCharacter * RideablePet::create( PCharacter *client ) const
{
    NPCharacter *horse;

    horse = create_mobile( ch->pIndexData );
    config( client, horse );

    char_to_room( horse, client->in_room );

    horse->add_follower( client );
    horse->leader = client;
    
    return horse;
}

bool RideablePet::purchase( Character *client, NPCharacter *keeper, const DLString &arguments, int )
{
    NPCharacter *horse;
    
    if (client->is_npc( ))
        return false;

    if (MOUNTED(client)) {
        client->pecho( "У тебя уже есть скакун." );
        return false;
    }

    if (!canAfford( client )) {
        client->pecho( "У тебя не хватает %N2, чтобы заплатить за это.", toCurrency( ).c_str( ) );
        return false;
    }

    if (getLevel( client ) - 5 > client->getModifyLevel( )) {
        ch->pecho("Тебе не хватит опыта справиться с этим скакуном.");
        return false;
    }

    deduct( client );
    horse = create( client->getPC( ) ); 
    client->setWaitViolence( 1 );
    
    interpret_fmt( client, "mount %s", horse->getNameP( ) );

    client->pecho("Наслаждайся своим скакуном.");
    act("%1$^C1 приобретает для верховой езды %2$C4.", client, horse, 0,TO_ROOM);
    return true;
}

int RideablePet::getOccupation( )
{
    return Pet::getOccupation( ) | (1 << OCC_BATTLEHORSE);
}

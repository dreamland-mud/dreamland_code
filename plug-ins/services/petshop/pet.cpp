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
#include "wrapperbase.h"

#include "dreamland.h"
#include "occupations.h"
#include "interp.h"
#include "act.h"
#include "../../anatolia/handler.h"
#include "merc.h"

#include "def.h"
#include "charutils.h"
#include "skill_utils.h"
#include "roomutils.h"
#include "move_utils.h"

GSN(haggle);
RELIG(fili);
PROF(druid);

/*----------------------------------------------------------------------
 * Pet
 *---------------------------------------------------------------------*/
Pet::Pet( ) 
{
}

void Pet::stopfol( Character *master ) 
{
    setLastCharmTime();
}

bool Pet::area( ) 
{
    Character *master = ch->master;
    
    // Auto-extract abandoned pets, unless in pet storage
    if (master) {
        // nothing to do, still charmed
        return false;
    }

    if (ch->reset_room == ch->in_room->vnum) {
        // nothing to do, still at pet storage room
        return false;
    }

    {
        // Druid spirits share the same behavior, but are not affected by auto-extract.
        // Their 'master' will be null once in switched state, causing them to disappear by mistake.
        WrapperBase *wbase = ch->getWrapper();
        if (wbase && wbase->hasField("bound_player"))
            return false;
    }
    

    DLString msg = fmt(0, "Брошенн%1$Gый|ый|ая|ые %1$C1 горько вздыха%1$nет|ют напоследок и ", ch);
    if (is_flying(ch))
        msg = msg + fmt(0, "улета%1$nет|ют ", ch);
    else if (RoomUtils::isWater(ch->in_room))
        msg = msg + fmt(0, "уплыва%1$nет|ют ", ch);
    else if (!CharUtils::hasLegs(ch))
        msg = msg + fmt(0, "уполза%1$nет|ют ", ch);
    else
        msg = msg + fmt(0, "уход%1$nит|ят ", ch);

    msg = msg + "восвояси.";
    ch->recho(msg.c_str());
    LogStream::sendNotice() 
        << "PET_EXTRACT: Extracting " << ch->getID() << " [" << ch->getNPC()->pIndexData->vnum << "] "
        << " in room " << ch->in_room->vnum << endl;

    extract_char( ch );
    return true;
}


bool Pet::purchase( Character *client, NPCharacter *keeper, const DLString &arguments, int )
{
    NPCharacter *pet;

    if (client->is_npc( ) || client->getPC( )->pet) {
        client->pecho( "У тебя уже есть один питомец." );
        return false;
    }
    
    if (client->getProfession( ) == prof_druid) {
        client->pecho( "Тебе омерзительна сама мысль о покупке этих несчастных порабощенных созданий." );
        client->pecho( "Ты надеешься, что если перестать покупать питомцев, то их просто выпустят на волю." );
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
    oldact("$c1 приобретает $C4.", client, 0, pet, TO_ROOM );
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
    affect_add_charm(pet);
    pet->comm = COMM_NOTELL;
    
    if (IS_SET( pet->act, ACT_NOALIGN ))
        pet->alignment = client->alignment;
    
    pet->setDescription( dlprintf( 
             "%s\r\nТы понимаешь, что %s будет защищать и следовать за {C%s{x до самой смерти.\n\r",     
             pet->getNPC( )->pIndexData->description, 
             pet->getNameP( '1' ).c_str( ), client->getNameP( '5' ).c_str( ) ) );
}

NPCharacter * Pet::create( PCharacter *client ) const
{
    NPCharacter *pet;

    pet = create_mobile( ch->pIndexData );
    config( client, pet ); 

    char_to_room( pet, client->in_room );
    
    client->pet = pet;
    follower_add( pet, client );
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
    
    pet->saving_throw = -level / 2;
    
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

    follower_add( horse, client );
    horse->leader = client;
    
    return horse;
}

bool RideablePet::purchase( Character *client, NPCharacter *keeper, const DLString &arguments, int )
{
    NPCharacter *horse;
    
    if (client->is_npc( ))
        return false;

    if (client->getProfession( ) == prof_druid) {
        client->pecho( "Тебе омерзительна сама мысль о покупке этих несчастных порабощенных созданий." );
        client->pecho( "Ты надеешься, что если перестать покупать питомцев, то их просто выпустят на волю." );
        return false;
    }
    
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
    
    interpret_fmt( client, "mount %s", horse->getNameC() );

    client->pecho("Наслаждайся своим скакуном.");
    oldact("$c1 приобретает для верховой езды $C4.", client, 0, horse, TO_ROOM );
    return true;
}

int RideablePet::getOccupation( )
{
    return Pet::getOccupation( ) | (1 << OCC_BATTLEHORSE);
}

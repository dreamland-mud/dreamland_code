/* $Id: stealquest.cpp,v 1.1.2.29.6.11 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */

#include "stealquest.h"
#include "objects.h"
#include "mobiles.h"

#include "questexceptions.h"

#include "player_utils.h"
#include "weapongenerator.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "object.h"

#include "merc.h"
#include "save.h"
#include "act.h"
#include "loadsave.h"
#include "vnum.h"
#include "def.h"

StealQuest::StealQuest( )
             : item( NULL )
{
}

void StealQuest::create( PCharacter *pch, NPCharacter *questman )
{
    Object *chest, *key;
    NPCharacter *thief, *victim;
    Room *hideaway;
    int time;
    DLString name;
    StealQuestRegistrator *reg = StealQuestRegistrator::getThis( );

    charName = pch->getName( );
    state = QSTAT_INIT;
    
    mode = number_range( -1, 3 );

    if (Player::isNewbie( pch ))
        mode = std::min( mode.getValue( ), 1 );

    try {
        item = getRandomItem( pch );
        victim = item->carried_by->getNPC( );
        thief = getRandomVictim( pch );
    
        if (Player::isNewbie( pch ) || chance(20))
        {
            key = chest = NULL;
            hideaway = NULL;
        }
        else {
            chest = createItem<HiddenChest>( reg->chests.randomVnum( ) );
            fillChest( pch, chest );
            key = createItem<LockPick>( chest->value2() );
            hideaway = findHideaway( pch, thief );
        }
        
        if (!isMobileVisible( thief, pch ) || !isMobileVisible( victim, pch ))
            mode++;
            
        if (!isItemVisible( item, pch ))
            mode++;
        
        if (Player::isNewbie( pch ) && mode > 1)
            throw QuestCannotStartException( );

        MobileQuestModel::assign<Robber>( thief );
        MobileQuestModel::assign<RobbedVictim>( victim ); 
        ItemQuestModel::assign<RobbedItem>( item );

    } catch (const QuestCannotStartException &e) {
        destroy( );
        throw e;
    }

    name = victim->getShortDescr(LANG_DEFAULT);
    name.upperFirstCharacter( );
    victimName = name;
    victimRoom = victim->in_room->getName();
    victimArea = victim->in_room->areaName();
    
    name = thief->getShortDescr(LANG_DEFAULT);
    name.upperFirstCharacter( );
    thiefName = name;
    thiefArea = thief->in_room->areaName();
    thiefRoom = thief->in_room->getName();
    thiefSex = thief->getSex( );

    itemName = item->getShortDescr(LANG_DEFAULT);
    itemWear.assign( item->wear_loc );
    chestRoom = getRoomHint( hideaway );

    obj_from_char( item );

    if (chest) {
        obj_to_obj_random( item, chest );
        obj_to_room( chest, hideaway );
        obj_to_char( key, thief );
    }
    else 
        obj_to_char( item, thief );
    

    time = number_fuzzy( 10 );
    setTime( pch, time );
    
    tell_raw( pch, questman, "У меня есть для тебя срочное поручение!" );
    
    switch (number_range( 1, 3 )) {
    case 1:  tell_fmt( "{W%3$#^C4{G обворовали, %3$P1 просит помочь в поимке негодяев.", 
                        pch, questman, victim );
             break;
    case 2:  tell_fmt( "{W%3$#^C1{G ста%3$Gло|л|ла жертвой грабителей и просит вернуть украденную вещь.", 
                        pch, questman, victim );
             break;
    case 3:  tell_fmt( "Воры ограбили {W%3$#C4{G, и теперь %3$P1 нуждается в твоей помощи.", 
                        pch, questman, victim );
             break;
    }

    tell_raw( pch, questman, "Пострадавшего ищи в районе {W%s{G ({W{hh%s{hx{G).", 
                  victim->in_room->getName(), victim->in_room->areaName().c_str() );
    tell_fmt("У тебя есть {Y%3$d{G мину%3$Iта|ты|т, чтобы добраться туда и узнать подробности.", 
              pch, questman, time );
    
    wiznet( "", "thief [%s] [%d], obj [%s], victim [%s] [%d], chest [%d], mode %d",
                thief->getNameP( '1' ).c_str(), thief->in_room->vnum,
                item->getShortDescr( '1', LANG_DEFAULT ).c_str( ),
                victim->getNameP( '1' ).c_str(), victim->in_room->vnum,
                (hideaway ? hideaway->vnum : 0),
                mode.getValue( ) );
}

void StealQuest::clear( Object *obj )
{
    ItemQuestModel::clear( obj );
    
    if (obj) {
        if (obj->carried_by 
            && obj->carried_by->is_npc( )
            && victimName ^ obj->carried_by->getNPC( )->getShortDescr(LANG_DEFAULT))
        {
            save_mobs( obj->carried_by->in_room );
        }
        else
            extract_obj( obj );
    }
}

void StealQuest::destroy( ) 
{
    clearMobile<RobbedVictim>( );
    clearMobile<Robber>( );
    clearItem<RobbedItem>( );
    destroyItem<HiddenChest>( );
    destroyItem<LockPick>( );
}

QuestReward::Pointer StealQuest::reward( PCharacter *ch, NPCharacter *questman ) 
{
    QuestReward::Pointer r( NEW );

    switch (mode.getValue( )) {
    case -1: r->points = number_range( 1, 5 ); break;
    case 0:  r->points = number_range( 5, 8 ); break;
    case 1:  r->points = number_range( 8, 12 ); break;
    case 2:  r->points = number_range( 12, 16 ); break;
    case 3:  r->points = number_range( 16, 22 ); break;
    default: r->points = number_range( 22, 26 ); break;
    }
    
    if (!chestRoom.getValue( ).empty( ))
        r->points += number_fuzzy( 10 );
    else    
        r->points += number_fuzzy( 3 );

    if (!Player::isNewbie(ch)) {
        r->points -= hint * 5;
    }
    
    r->gold = number_fuzzy( r->points );
    r->wordChance = r->points;
    r->scrollChance = number_range( 5, mode * 4 );

    if (chance( mode ))
        r->prac = number_range( 1, 3 );
    
    if (ch->getClan( )->isDispersed( )) 
        r->points *= 2;
    else
        r->clanpoints = r->points;
    
    r->points = std::max( 5, r->points );
    r->clanpoints = std::max( 0, r->clanpoints );
    r->exp = (r->points + r->clanpoints) * 10;

    return r;
}

void StealQuest::info( std::ostream &buf, PCharacter *ch ) 
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "У " << russian_case( victimName, '2' ) << " какие-то неприятности." << endl
            << "Пострадавший ждет тебя в районе '" 
            << victimRoom << "' ({hh" << victimArea << "{hx)." << endl;
        break;

    case QSTAT_HUNT_ROBBER:
        buf << "Тебе стало известно, что у " << russian_case( victimName, '2' ) 
            << " украли " << russian_case( itemName, '4' ) << ". " << endl
            << "Вор - " << russian_case( thiefName, '1' ) 
            << ", скорее всего скрывается в районе '" << thiefRoom << "' ({hh" << thiefArea << "{hx)." << endl;

            if (!chestRoom.getValue( ).empty( ))
                buf << "По слухам, награбленное добро спрятано где-то около '" << chestRoom << "'" 
                    << ", ключ от нычки ищи у вора." << endl;

            buf << "Пострадавший ждет тебя около '" 
                << victimRoom << "' ({hh" << victimArea << "{hx)." << endl;

        break;

    case QSTAT_FINISHED:
        buf << "Твое задание выполнено!" << endl
            << "Вернись за вознаграждением до того, как выйдет время." << endl;
        break;
    }
}

void StealQuest::shortInfo( std::ostream &buf, PCharacter *ch )
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "Узнать, что случилось у " << russian_case( victimName, '2') << " в "
            << victimRoom << " (" << victimArea << ").";
        break;

    case QSTAT_HUNT_ROBBER:
        buf << "Вернуть " << russian_case( itemName, '4' ) << " " << russian_case( victimName, '3' ) << ". "
            << "Вор, " << russian_case( thiefName, '1' ) << ", скрывается в " 
            << thiefRoom << " (" << thiefArea << ")";

            if (!chestRoom.getValue( ).empty( ))
                buf << ", награбленное прячет около " << chestRoom << ".";
            else 
                buf << ".";
        break;

    case QSTAT_FINISHED:
        buf << "Вернуться к квестору за наградой.";
        break;
    }
}

bool StealQuest::isComplete( )
{
    return (state == QSTAT_FINISHED);
}

void StealQuest::helpMessage( ostringstream &buf )
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        buf << "До " << russian_case( victimName, '2' )
            << " ты можешь добраться, следуя по такому пути: ";
        break;

    case QSTAT_HUNT_ROBBER:
        buf << "Ты можешь отыскать " << russian_case( thiefName, '4' )
            << ", следуя по такому пути: ";
        break;
    }
}

Room * StealQuest::helpLocation( ) 
{
    switch (state.getValue( )) {
    case QSTAT_INIT:
        return findMobileRoom<RobbedVictim>( );
        
    case QSTAT_HUNT_ROBBER:
        return findMobileRoom<Robber>( );
    }

    return NULL;
}

bool StealQuest::checkItem( PCharacter *pch, Object *obj )
{
    NPCharacter *victim;

    if (!obj->carried_by || !obj->carried_by->is_npc( ))
        return false;
    
    victim = obj->carried_by->getNPC( );
    
    if (!checkMobileClient( pch, victim ))
        return false;
    
    if (!victim->can_see( obj ))
        return false;

    // Hands almost full: may not be possible to give the item back.
    if (victim->carry_number >= Char::canCarryNumber(victim))
        return false;

    if (Char::getCarryWeight(victim) >= Char::canCarryWeight(victim))
        return false;

    return ItemQuestModel::checkItem( pch, obj );
}

bool StealQuest::checkMobileVictim( PCharacter *pch, NPCharacter *mob )
{
    int level_diff, min_diff;

    if (!VictimQuestModel::checkMobileVictim( pch, mob )) 
        return false;

    if (mob->in_room->area == item->carried_by->in_room->area)
        return false;;

    level_diff = mob->getRealLevel( ) - pch->getModifyLevel( );
    min_diff = (mob->getRealLevel( ) < 50 ? -10 : -20);
    
    if (level_diff > 15 || level_diff < min_diff) 
        return false;
    
    if ((mode == -1 && level_diff > -10) || level_diff >= 5 * mode)
        return false;

    if (!isThief( mob ))
        return false;
    
    return true;
}

bool StealQuest::checkMobileClient( PCharacter *pch, NPCharacter *mob )
{
    if (!ClientQuestModel::checkMobileClient( pch, mob )) 
        return false;

    if (isThief( mob ))
        return false;
        
    if (mob->carry_number > Char::canCarryNumber(mob))
        return false;

    if (Char::getCarryWeight(mob) > Char::canCarryWeight(mob))
        return false;

    return true;
}

bool StealQuest::isThief( NPCharacter *mob )
{
    if (IS_SET( mob->act, ACT_THIEF ))
        return true;

    if (mob->spec_fun.name == "spec_thief")
        return true;
  
    if (StealQuestRegistrator::getThis( )->thiefs.hasName( mob ))
        return true;
        
    return false;
}


void StealQuest::fillChest( PCharacter *pch, Object *chest )
{
    Object *obj;
    VnumList objects;
    
    /* барахло */
    for (obj = object_list; obj; obj = obj->next) {
        if (IS_SET( obj->pIndexData->area->area_flag, AREA_NOQUEST ))
            continue;
        if (obj->pIndexData->reset_num <= 0)
            continue;
        if (obj->pIndexData->cost >= 10000) 
            continue;
        if (!IS_SET( obj->wear_flags, ITEM_TAKE ))
            continue;

        if (isBonus( obj->pIndexData, pch ) 
            && number_range( 1, obj->pIndexData->count ) == 1)
        {
            objects.push_back( obj->pIndexData->vnum );
        }
    }
    
    if (!objects.empty( )) {
        int count = number_range( 0, 15 );

        for ( ; count; count--) 
            if (( obj = objects.randomItem( ) ))
                obj_to_obj( obj, chest );
    }
    
    /* ништяки */
    if (chance( 1 )) 
        if (( obj = StealQuestRegistrator::getThis( )->bonuses.randomItem( ) ))
            obj_to_obj( obj, chest );

    obj_to_obj( Money::create( 
                    number_range( 0, pch->getModifyLevel( ) ), 
                    number_range( 0, 1000 ) ), chest );

    // Create tier 4-5 random weapon for this player.
    Object *weapon = create_object(get_obj_index(OBJ_VNUM_WEAPON_STUB), 0);
    obj_to_obj(weapon, chest);
    weapon->level = pch->getModifyLevel();
    WeaponGenerator()
        .item(weapon)
        .alignment(pch->alignment)
        .player(pch)
        .randomTier(4)
        .randomizeAll();
}

bool StealQuest::isBonus( OBJ_INDEX_DATA *pObjIndex, PCharacter *pch )
{
    int olevel = pObjIndex->level;
    int mlevel = pch->getModifyLevel( );

    if (olevel > mlevel + mlevel / 10 || olevel < mlevel - mlevel / 5) 
        return false;

    if (pObjIndex->limit != -1 || olevel > ANGEL) 
        return false;
    
    return true;
}

Room * StealQuest::findHideaway( PCharacter *pch, NPCharacter *thief )
{
    RoomVector places, places1, places2;
    RoomVector::iterator r;

    for (auto &r: thief->in_room->area->rooms) {
        if (checkRoom( pch, r.second ) && !RoomUtils::isWaterOrAir(r.second))
            places.push_back( r.second );
    }
    
    for (r = places.begin( ); r != places.end( ); r++) {
        EXIT_DATA *e;
        int cnt = 0;

        for (int d = 0; d < DIR_SOMEWHERE; d++) {
            e = (*r)->exit[d];

            if (e && e->u1.to_room)
                cnt++;
        }
        
        if (cnt == 1)
            places1.push_back( *r );
        if (cnt == 2)
            places2.push_back( *r );
    }
    
    if (!places1.empty( ))
        return places1[number_range( 0, places1.size( ) - 1 )];
    if (!places2.empty( ))
        return places2[number_range( 0, places2.size( ) - 1 )];
    if (!places.empty( ))
        return places[number_range( 0, places.size( ) - 1 )];
    
    throw QuestCannotStartException( );
}

DLString StealQuest::getRoomHint( Room * room, Room *from, int depth )
{
    if (!room)
        return "";

    if (depth >= 2) 
        return room->getName();

    for (int d = 0; d < DIR_SOMEWHERE; d++) {
        Room *r;

        if (!room->exit[d])
            continue;
        
        if (!( r = room->exit[d]->u1.to_room ))
            continue;
        
        if (r == from)
            continue;

        if (!r->isCommon() || IS_SET(r->room_flags, ROOM_MANSION))
            continue;
        
        for (int i = 0; i < DIR_SOMEWHERE; i++)
            if (r->exit[i] && r->exit[i]->u1.to_room == room)
                return getRoomHint( r, room, depth + 1 );
    }

    return room->getName();
}

/*
 * StealQuestRegistrator
 */
StealQuestRegistrator * StealQuestRegistrator::thisClass = NULL;

StealQuestRegistrator::StealQuestRegistrator( )
{
    thisClass = this;
}

StealQuestRegistrator::~StealQuestRegistrator( )
{
    thisClass = NULL;
}


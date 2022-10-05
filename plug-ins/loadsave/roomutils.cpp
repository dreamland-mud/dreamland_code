#include "character.h"
#include "room.h"
#include "roomutils.h"
#include "merc.h"
#include "def.h"
#include "loadsave.h"
#include "liquid.h"

LIQ(none);

bool RoomUtils::isWater(Room *target)
{
    int s = target->getSectorType();

    return s == SECT_WATER_NOSWIM 
        || s == SECT_WATER_SWIM
        || s == SECT_UNDERWATER;
}

bool RoomUtils::isWaterOrAir(Room *target)
{
    return isWater(target) || target->getSectorType() == SECT_AIR;
}

bool RoomUtils::isOutside(Room *target)
{
    return !IS_SET(target->room_flags, ROOM_INDOORS) && target->getSectorType() != SECT_UNDERWATER;
}

bool RoomUtils::isOutside(Character *ch)
{
    return isOutside(ch->in_room);
}

bool RoomUtils::hasWaterParticles(Room *target)
{
    if ( (isWater(target)) ||
         (IS_SET(target->room_flags, ROOM_NEAR_WATER)) ||
         (isOutside(target) && weather_info.sky >= SKY_RAINING) ||
         (get_obj_room_type(target, ITEM_FOUNTAIN) != 0) ) {
        return true;
    }
    return false;
}

bool RoomUtils::isNature(Room *target)
{
    int s = target->getSectorType();

    return s == SECT_FIELD
        || s == SECT_FOREST
        || s == SECT_MOUNTAIN
        || s == SECT_HILLS;
}

bool RoomUtils::hasDust(Room *target)
{
    return !isWater(target) && target->getSectorType() != SECT_AIR;
}

bool RoomUtils::hasParticles(Room *target)
{
    int s = target->getSectorType();
    return s != SECT_UNDERWATER 
        && s != SECT_INSIDE 
        && isOutside(target);
}


bool RoomUtils::isRandom(Room *room) 
{
    for(auto &pReset: room->pIndexData->resets)
        switch (pReset->command) {
            case 'R': 
                return true;
        }

    return false;
}

std::list<Room *> RoomUtils::adjancentRooms(Room *source)
{
    map<int, bool> roomMarks;
    list<Room *> result;
    
    for (int door = 0; door < DIR_SOMEWHERE; door++) {
        EXIT_DATA *pexit;
        Room *room;

        if (!( pexit = source->exit[door] ))
            continue;
            
        if (!( room = pexit->u1.to_room ))
            continue;
        
        if (room == source)
            continue;

        if (roomMarks[room->vnum] == true)
            continue;
            
        roomMarks[room->vnum] = true;
        result.push_back(room);
    }

    return result;
}
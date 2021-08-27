#include "roomutils.h"
#include "character.h"
#include "room.h"
#include "merc.h"
#include "def.h"

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
    return !IS_SET(target->room_flags, ROOM_INDOORS);
}

bool RoomUtils::isOutside(Character *ch)
{
    return isOutside(ch->in_room);
}

bool RoomUtils::hasWaterParticles(Room *target)
{
    if (isWater(target))
        return true;

    if (IS_SET(target->room_flags, ROOM_NEAR_WATER))
        return true;

    if (isOutside(target) && weather_info.sky >= SKY_RAINING)
        return true;

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

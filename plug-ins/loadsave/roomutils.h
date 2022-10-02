#ifndef __ROOMUTILS_H__
#define __ROOMUTILS_H__

#include <list>

class Room;
class Character;

namespace RoomUtils {
    bool hasParticles(Room *r);
    bool hasDust(Room *r);
    bool isNature(Room *r);
    bool hasWaterParticles(Room *r);
    bool isOutside(Room *r);
    bool isOutside(Character *ch);
    bool isWater(Room *r);
    bool isWaterOrAir(Room *r);
    bool isRandom(Room *r);

    // Return a list of unique rooms that are accessible from this one.
    std::list<Room *> adjancentRooms(Room *r);
}

#endif // __ROOMUTILS_H__
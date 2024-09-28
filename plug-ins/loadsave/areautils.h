#ifndef AREAUTILS_H
#define AREAUTILS_H

class AreaIndexData;
class PCMemoryInterface;
struct mob_index_data;
class RoomIndexData;

namespace AreaUtils {
    // Create a "playername.area" zone for this player, with 100 vnums by default.
    AreaIndexData *createFor(PCMemoryInterface *player);

    // Calculate next available vnum range for a sandbox area.
    int findMinSandboxVnum();

    mob_index_data * findFirstMob(AreaIndexData *pArea);

    RoomIndexData * findFirstRoom(AreaIndexData *pArea);

};

#endif

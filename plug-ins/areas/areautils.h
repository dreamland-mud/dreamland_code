#ifndef AREAUTILS_H
#define AREAUTILS_H

class AreaIndexData;
class PCMemoryInterface;
struct mob_index_data;
class RoomIndexData;
class AreaHelp;
class DLString;

namespace AreaUtils {
    // Create a "playername.area" zone for this player, with 100 vnums by default.
    AreaIndexData *createFor(PCMemoryInterface *player);

    // Calculate next available vnum range for a sandbox area.
    int findMinSandboxVnum();

    mob_index_data * findFirstMob(AreaIndexData *pArea);

    RoomIndexData * findFirstRoom(AreaIndexData *pArea);

    // Create self-help article for this area.
    AreaHelp * createHelp(AreaIndexData* pArea);

    // Find area with given vnum
    AreaIndexData *getByVnum(int vnum);

    // Find area with given file name 
    AreaIndexData *getByFileName(const DLString &filename);

    // Find area by vnum or file name
    AreaIndexData *lookup(const DLString &arg);

};

#endif

#ifndef AREAUTILS_H
#define AREAUTILS_H

class AreaIndexData;
class PCMemoryInterface;

namespace AreaUtils {
    // Create a "playername.area" zone for this player, with 100 vnums by default.
    AreaIndexData *createFor(PCMemoryInterface *player);

    // Calculate next available vnum range for a sandbox area.
    int findMinSandboxVnum();
};

#endif

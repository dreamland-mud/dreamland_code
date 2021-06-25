#include <array>
#include <functional>
#include "areautils.h"
#include "pcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#define SANDBOX_VNUM_START 100000
#define SANDBOX_SIZE 99

int AreaUtils::findMinSandboxVnum()
{
    // Populate a list of all low/high vnum ranges.
    vector<pair<int, int>> ranges;
    for (auto &pArea: areaIndexes)
        ranges.push_back(make_pair(pArea->min_vnum, pArea->max_vnum));

    // Sort all ranges in ascending order by min_vnum.
    sort(begin(ranges), end(ranges),
         [](auto &a, auto &b) { return a.first < b.first; });

    // Find uppermost range, ensure it's outside of the sandbox vnum boundaries.
    int top_vnum = ranges.back().second;
    int my_vnum = max(SANDBOX_VNUM_START, top_vnum + 1);

    return my_vnum;
}

AreaIndexData * AreaUtils::createFor(PCMemoryInterface *player)
{
    DLString filename = player->getName().toLower() + ".are";

    AreaIndexData *a = new AreaIndexData;    
    a->vnum = top_area++;
    a->area_file = new_area_file(filename.c_str());
    a->area_file->area = a;

    areaIndexes.push_back(a);

    DLString areaname = "Зона " + player->getNameP('2');
    a->name = str_dup(areaname.c_str());
    a->authors = str_dup(player->getNameP('1').c_str());
    a->security = 9;
    a->low_range = 1;
    a->high_range = 100;
    a->min_vnum = findMinSandboxVnum();
    a->max_vnum = a->min_vnum + SANDBOX_SIZE;

    a->create();
    a->changed = true;

    return a;
}


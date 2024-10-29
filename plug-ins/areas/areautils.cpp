#include <array>
#include <functional>
#include "areautils.h"
#include "pcharacter.h"
#include "room.h"
#include "areahelp.h"
#include "string_utils.h"
#include "merc.h"
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

    DLString areaname = "Зон|а|ы|е|у|ой|е " + player->getNameP('2');
    a->name[RU] = areaname;
    a->authors = player->getNameP('1');
    a->security = 9;
    a->low_range = 1;
    a->high_range = 100;
    a->min_vnum = findMinSandboxVnum();
    a->max_vnum = a->min_vnum + SANDBOX_SIZE;

    a->create();
    a->changed = true;

    AreaHelp::Pointer help = createHelp(a);
    XMLPersistent<HelpArticle> phelp(help.getPointer());
    a->helps.push_back(phelp);

    return a;
}

mob_index_data* AreaUtils::findFirstMob(AreaIndexData *pArea)
{
    for (int i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        mob_index_data *pMob = get_mob_index(i);
        if (pMob)
            return pMob;
    }

    return 0;
}

RoomIndexData * AreaUtils::findFirstRoom(AreaIndexData *pArea)
{
    if (pArea->roomIndexes.empty())
        return 0;
        
    return pArea->roomIndexes.begin()->second;    
}

AreaHelp * AreaUtils::createHelp(AreaIndexData *pArea)
{
    AreaHelp::Pointer ahelp(NEW);    

    DLString aname = pArea->getName().colourStrip().quote();
    ahelp->keyword[RU] = aname;
    ahelp->setAreaIndex(pArea);

    ahelp->setID(
        helpManager->getLastID() + 1
    );
    
    helpManager->unregistrate(AreaHelp::Pointer(ahelp));
    helpManager->registrate(AreaHelp::Pointer(ahelp));
    
    return *ahelp;
}
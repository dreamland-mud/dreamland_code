#include "xmlattributevisited.h"
#include "room.h"
#include "area.h"
#include "idcontainer.h"
#include "merc.h"
#include "def.h"

using namespace Scripting;

const DLString XMLAttributeVisited::TYPE = "XMLAttributeVisited";

XMLAttributeVisited::XMLAttributeVisited() : parsed(false)
{
}

int XMLAttributeVisited::count() const
{
    parse();
    return vnums.size();
}

void XMLAttributeVisited::parse() const
{
    if (parsed)
        return;
    parsed = true;
    vnums.clear();

    const DLString &s = rooms.getValue();
    size_t pos = 0;
    while (pos < s.size()) {
        size_t comma = s.find(',', pos);
        if (comma == DLString::npos)
            comma = s.size();
        DLString part = s.substr(pos, comma - pos);
        size_t dash = part.find('-');
        int from = 0, to = 0;
        if (dash == DLString::npos) {
            from = to = atoi(part.c_str());
        } else {
            from = atoi(part.substr(0, dash).c_str());
            to = atoi(part.substr(dash + 1).c_str());
        }
        // A corrupt range can't blow the set up (no world has 65536 rooms
        // in one run of vnums).
        if (to - from > 65535)
            to = from + 65535;
        for (int v = from; v > 0 && v <= to; v++)
            vnums.insert(v);
        pos = comma + 1;
    }
}

void XMLAttributeVisited::store()
{
    ostringstream buf;
    int from = -1, prev = -1;
    for (int v : vnums) {
        if (from < 0) {
            from = prev = v;
            continue;
        }
        if (v == prev + 1) {
            prev = v;
            continue;
        }
        if (buf.tellp() > 0)
            buf << ",";
        if (from == prev) buf << from; else buf << from << "-" << prev;
        from = prev = v;
    }
    if (from >= 0) {
        if (buf.tellp() > 0)
            buf << ",";
        if (from == prev) buf << from; else buf << from << "-" << prev;
    }
    rooms.setValue(buf.str());
}

bool XMLAttributeVisited::visit(int vnum)
{
    if (vnum <= 0)
        return false;
    parse();
    if (!vnums.insert(vnum).second)
        return false;
    store();
    return true;
}

bool XMLAttributeVisited::isOrdinary(RoomIndexData *pRoom)
{
    if (!pRoom || !pRoom->areaIndex)
        return false;
    if (IS_SET(pRoom->areaIndex->area_flag,
               AREA_SYSTEM | AREA_HIDDEN | AREA_CLAN | AREA_MANSION | AREA_WIZLOCK))
        return false;
    if (IS_SET(pRoom->room_flags,
               ROOM_GODS_ONLY | ROOM_IMP_ONLY | ROOM_NEWBIES_ONLY | ROOM_NOWHERE | ROOM_MANSION))
        return false;
    // No way out at all: storage rooms for shop pets, voids. Nobody walks in.
    for (int d = 0; d < 6; d++)
        if (pRoom->exit[d])
            return true;
    return !pRoom->extra_exits.empty();
}

// Recounted only when rooms are added or removed: a flag edit in OLC shows up
// at the next boot. Good enough for a progress bar.
int XMLAttributeVisited::ordinaryTotal()
{
    static size_t countedSize = 0;
    static int total = 0;

    if (countedSize != roomIndexMap.size()) {
        countedSize = roomIndexMap.size();
        total = 0;
        for (auto &r : roomIndexMap)
            if (isOrdinary(r.second))
                total++;
    }
    return total;
}

int XMLAttributeVisited::countOrdinary() const
{
    parse();
    int n = 0;
    for (int v : vnums) {
        auto r = roomIndexMap.find(v);
        if (r != roomIndexMap.end() && isOrdinary(r->second))
            n++;
    }
    return n;
}

Register XMLAttributeVisited::toRegister() const
{
    Register attrReg = Register::handler<IdContainer>();
    IdContainer *c = attrReg.toHandler().getDynamicPointer<IdContainer>();
    c->setField(IdRef("visited"), countOrdinary());
    c->setField(IdRef("total"), ordinaryTotal());
    return attrReg;
}

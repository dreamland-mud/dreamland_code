#include "xmlpvpattribute.h"
#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "lex.h"
#include "regcontainer.h"

using namespace Scripting;

const time_t XMLPvpAttribute::REPEAT_WINDOW = 24 * 60 * 60;

bool XMLPvpAttribute::record(const DLString &victimName, time_t when)
{
    Victims::iterator v = victims.find(victimName);

    // Older or too recent timestamps are refused, which also makes a replay of
    // the same history (sorted by time) a no-op the second time around.
    if (v != victims.end() && v->second.count > 0
            && when - (time_t)v->second.last.getValue() < REPEAT_WINDOW)
        return false;

    XMLPvpVictim &victim = victims[victimName];
    victim.count = victim.count + 1;
    victim.last.setValue(when);
    kills = kills + 1;
    return true;
}

Scripting::Register XMLPvpAttribute::toRegister() const
{
    Register pvpReg = Register::handler<IdContainer>();
    IdContainer *pvp = pvpReg.toHandler().getDynamicPointer<IdContainer>();

    Register victimsReg = Register::handler<RegContainer>();
    RegContainer *victimsContainer = victimsReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto &v: victims)
        victimsContainer->setField(v.first, v.second.count.getValue());

    pvp->setField(IdRef("kills"), kills.getValue());
    pvp->setField(IdRef("unique"), (int)victims.size());
    pvp->setField(IdRef("victims"), victimsReg);

    return pvpReg;
}

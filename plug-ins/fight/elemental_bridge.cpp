/* Hand-over from C++ combat code to the Fenia elemental engine.
 * See elemental_bridge.h for what this replaced and why.
 *
 * dreamland-mud, 2026
 */
#include "elemental_bridge.h"

#include "register-impl.h"
#include "wrapperbase.h"

#include "character.h"
#include "room.h"

#include "fight_exception.h"
#include "fenia_utils.h"
#include "feniamanager.h"

#include "def.h"

/**
 * Both entries below are on combat paths, so they use gprog_nocatch and let a
 * VictimDeathException keep travelling.
 *
 * Plain gprog() catches ::Exception, and VictimDeathException derives from it
 * (fight_exception.h). A Fenia handler that killed the victim would therefore
 * have its kill filed as a script error and the melee round would carry on
 * hammering a corpse. Everything else is still croaked, so a genuine script bug
 * reaches the logs and the immortals instead of unwinding the fight.
 */
static void elemental_invoke(const char *trigger, const char *element,
                             Character *source, Character *victim, Room *room, int power)
{
    try {
        if (room)
            gprog_nocatch(trigger, "sCRi", element, source, room, power);
        else
            gprog_nocatch(trigger, "sCCi", element, source, victim, power);
    }
    catch (const VictimDeathException &) {
        throw;
    }
    catch (const ::Exception &e) {
        FeniaManager::getThis()->croak(0, Scripting::Register(trigger), e);
    }
}

void elemental_effect(const char *element, Character *source, Character *victim, int power)
{
    if (!element || !source || !victim)
        return;

    elemental_invoke("onElementalEffect", element, source, victim, 0, power);
}

void elemental_effect_room(const char *element, Character *source, Room *room, int power)
{
    if (!element || !source || !room)
        return;

    elemental_invoke("onElementalEffectRoom", element, source, 0, room, power);
}

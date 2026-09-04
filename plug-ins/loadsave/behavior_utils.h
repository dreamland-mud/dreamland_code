#ifndef BEHAVIOR_UTILS_H
#define BEHAVIOR_UTILS_H

#include <list>
#include "register-decl.h"

class Object;
class Character;
class Room;
class DLString;
class GlobalBitvector;

/** Directly call a trigger with arguments defined for these behaviors. */
list<Scripting::Register> behavior_trigger_with_result(GlobalBitvector &behaviors, const DLString &trigType, const Scripting::RegisterList &trigArgs);

/** Cheap presence gate: does any behavior in the set define on<trig> or post<trig>?
 *  Lets a hot per-tick dispatcher skip behavior_trigger's arg-building when nothing
 *  handles the trigger. Reads the live guts trigger map (the same predicate the
 *  dispatch fires on), so there is no cache and nothing to invalidate. Pass the two
 *  trigger ids pre-resolved (e.g. a hoisted static IdRef) to avoid a lex lookup per call. */
bool behaviors_have_trigger(GlobalBitvector &behaviors, const Scripting::Register &onId, const Scripting::Register &postId);

/** For each behavior assigned in OBJ_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in MOB_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Character *ch, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in RoomIndexData call trigType trigger with arguments. */
bool behavior_trigger(Room *room, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in RoomIndexData call trigType and return the resulting string. */
DLString behavior_trigger_str(Room *room, const DLString &trigType, const char *fmt, ...);

#endif
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

/** For each behavior assigned in OBJ_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in MOB_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Character *ch, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in RoomIndexData call trigType trigger with arguments. */
bool behavior_trigger(Room *room, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in RoomIndexData call trigType and return the resulting string. */
DLString behavior_trigger_str(Room *room, const DLString &trigType, const char *fmt, ...);

#endif
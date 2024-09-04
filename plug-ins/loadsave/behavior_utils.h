#ifndef BEHAVIOR_UTILS_H
#define BEHAVIOR_UTILS_H

class Object;
class Character;
class Room;
class DLString;

/** For each behavior assigned in OBJ_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...);

#endif
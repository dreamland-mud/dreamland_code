#ifndef BEHAVIOR_UTILS_H
#define BEHAVIOR_UTILS_H

#include <list>
#include "register-decl.h"

class Object;
class Character;
class Room;
class DLString;
class GlobalBitvector;
class Behavior;
struct mob_index_data;

/** Directly call a trigger with arguments defined for these behaviors. */
list<Scripting::Register> behavior_trigger_with_result(GlobalBitvector &behaviors, const DLString &trigType, const Scripting::RegisterList &trigArgs);

/** Cheap presence gate: does any behavior in the set define on<trig> or post<trig>?
 *  Lets a hot per-tick dispatcher skip behavior_trigger's arg-building when nothing
 *  handles the trigger. Reads the live guts trigger map (the same predicate the
 *  dispatch fires on), so there is no cache and nothing to invalidate. Pass the two
 *  trigger ids pre-resolved (e.g. a hoisted static IdRef) to avoid a lex lookup per call. */
bool behaviors_have_trigger(GlobalBitvector &behaviors, const Scripting::Register &onId, const Scripting::Register &postId);

/** Behavior an object answers 'use' with by its kind rather than its prototype
 *  (any axe chops wood), or null. behavior_trigger(obj, "Use") adds it. */
Behavior *object_implicit_use_behavior(Object *obj);

/** Does a pet of this prototype take its buyer's level? True for the 'rat'
 *  behavior and for 'pet' with props pet.adaptive set (bool, non-zero number
 *  or the string "true" / "1"). */
bool mob_index_pet_adaptive(struct mob_index_data *pMob);

/** For each behavior assigned in OBJ_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in MOB_INDEX_DATA call trigType trigger with arguments. */
bool behavior_trigger(Character *ch, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in MOB_INDEX_DATA call trigType trigger, for its
 *  side effects only: results are dropped, never converted, nothing is thrown. */
void behavior_notify(Character *ch, const DLString &trigType, const char *fmt, ...);

/** Did a script call made after the snapshot extract this character?
 *  An NPC extraction ends in init(), which resets extracted, in_room and the ID,
 *  and the object goes back to the pool for the next mob created. So a char that
 *  had a room and lost it is gone, and so is one whose ID changed (the handler
 *  extracted it, then created a mob that got the same object). Once gone() is
 *  true only the old pointer value may be used, never the object.
 *  A char already being extracted when the watch is armed (stopfol fired from
 *  extract_char) only counts as gone once it is recycled, so its handlers still
 *  run; countExtracting = true makes any extracted flag count, the ai_trigger rule. */
class ExtractWatch {
public:
    ExtractWatch(Character *ch, bool countExtracting = false);
    bool gone() const;

private:
    Character *ch;
    Room *room;
    long long id;
    bool wasExtracting;
};

/** For each behavior assigned in RoomIndexData call trigType trigger with arguments. */
bool behavior_trigger(Room *room, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in RoomIndexData call trigType and return the resulting string. */
DLString behavior_trigger_str(Room *room, const DLString &trigType, const char *fmt, ...);

/** For each behavior assigned in MOB_INDEX_DATA call trigType and return the concatenated strings. */
DLString behavior_trigger_str(Character *ch, const DLString &trigType, const char *fmt, ...);

#endif
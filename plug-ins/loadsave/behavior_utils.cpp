#include "core/behavior/behavior_utils.h"
#include "fenia_utils.h"
#include "behavior.h"
#include "lex.h"
#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "object.h"
#include "npcharacter.h"
#include "room.h"
#include "itemflags.h"

list<Register> behavior_trigger_with_result(GlobalBitvector &behaviors, const DLString &trigType, const RegisterList &trigArgs)
{
	list<Scripting::Register> regList;

	if (behaviors.empty())
		return regList;

	try {
		// Execute onXXX, postXXX triggers fro all behaviors associated with this item.
		for (int &bhvIndex: behaviors.toArray()) {
			Behavior *bhv = behaviorManager->find(bhvIndex);
			WrapperBase *bhvWrapper = bhv->getWrapper();

			// TODO: handle multiple onUse and Behavior::cmd.
			if (bhvWrapper) {
				Scripting::Register result;

				fenia_trigger(result, trigType, trigArgs, bhvWrapper, 0);

				if (result.type != Register::NONE)
					regList.push_back(result);
			}
		}

	} catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
		DLString methodId = "behavior on" + trigType;
        FeniaManager::getThis()->croak(0, Scripting::Register(methodId), e);
    }

	return regList;
}


// Presence gate for hot per-tick dispatchers. Mirrors the behavior iteration in
// behavior_trigger_with_result, but only asks 'does a behavior define this trigger?'
// via the same triggerFunction (guts lookup) the dispatch fires on -- no arg-building,
// no cache, never stale. Ids come in pre-resolved so there is no lex lookup per call.
bool behaviors_have_trigger(GlobalBitvector &behaviors, const Scripting::Register &onId, const Scripting::Register &postId)
{
	if (behaviors.empty())
		return false;

	for (int &bhvIndex: behaviors.toArray()) {
		Behavior *bhv = behaviorManager->find(bhvIndex);
		WrapperBase *bhvWrapper = bhv->getWrapper();
		if (!bhvWrapper)
			continue;

		Scripting::Register prog;
		if (bhvWrapper->triggerFunction(onId, prog) || bhvWrapper->triggerFunction(postId, prog))
			return true;
	}

	return false;
}


// Invoke 'trigType' trigger on all elements of 'behaviors' array, with args passed in the 'ap' var args.
// Returns a list of all non-null result registers.
static list<Register> behavior_trigger_with_result(
	GlobalBitvector &behaviors, 
	const DLString &trigType, 
	const char *fmt, 
	va_list ap)
{
	list<Scripting::Register> regList;

	if (behaviors.empty())
		return regList;

	try {
		RegisterList trigArgs;

		// Collect all arguments
		WrapperBase::triggerArgs(trigArgs, fmt, ap);

		return behavior_trigger_with_result(behaviors, trigType, trigArgs);

	} catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
		DLString methodId = "behavior on" + trigType;
        FeniaManager::getThis()->croak(0, Scripting::Register(methodId), e);
    }

	return regList;
}


DLString behavior_trigger_str(Room *room, const DLString &trigType, const char *fmt, ...) 
{
	va_list ap;    
	va_start(ap, fmt);

	auto regList = behavior_trigger_with_result(room->pIndexData->behaviors, trigType, fmt, ap);
	
	va_end(ap);
	
	return reglist_to_str(regList);
}

bool behavior_trigger(Room *room, const DLString &trigType, const char *fmt, ...) 
{
	va_list ap;    
	va_start(ap, fmt);

	auto regList = behavior_trigger_with_result(room->pIndexData->behaviors, trigType, fmt, ap);
	
	va_end(ap);
	
	return reglist_to_bool(regList);
}

DLString behavior_trigger_str(Character *ch, const DLString &trigType, const char *fmt, ...) 
{
	if (!ch->is_npc())
		return DLString::emptyString;

	va_list ap;    
	va_start(ap, fmt);

	auto regList = behavior_trigger_with_result(ch->getNPC()->pIndexData->behaviors, trigType, fmt, ap);

	va_end(ap);
	return reglist_to_str(regList);
}

bool behavior_trigger(Character *ch, const DLString &trigType, const char *fmt, ...) 
{
	if (!ch->is_npc())
		return false;

	va_list ap;    
	va_start(ap, fmt);

	auto regList = behavior_trigger_with_result(ch->getNPC()->pIndexData->behaviors, trigType, fmt, ap);

	va_end(ap);
	return reglist_to_bool(regList);
}

// Behaviors an object has by its kind rather than by its prototype: every
// axe-class weapon (raw value0, the class it IS) is a lumberjack axe, so random,
// hero and crafted axes chop without listing vnums. The behavior script decides
// what each axe may do (two-handers fell trees, any axe hacks wood).
// Returns null when the object has nothing beyond its prototype's list.
static Behavior *implicit_behavior(Object *obj)
{
	if (obj->item_type != ITEM_WEAPON || obj->value0() != WEAPON_AXE)
		return 0;

	return behaviorManager->findExisting("axe lumberjack");
}

bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...)
{
	// A trigger fired earlier in the same tick can extract this very object
	// (Object::extract nulls pIndexData); bail before the deref segfaults.
	if (!obj->pIndexData)
		return false;

	va_list ap;
	va_start(ap, fmt);

	list<Register> regList;
	Behavior *implicit = implicit_behavior(obj);

	if (implicit && !obj->pIndexData->behaviors.isSet(*implicit)) {
		GlobalBitvector behaviors = obj->pIndexData->behaviors;
		behaviors.set(*implicit);
		regList = behavior_trigger_with_result(behaviors, trigType, fmt, ap);
	} else {
		regList = behavior_trigger_with_result(obj->pIndexData->behaviors, trigType, fmt, ap);
	}

	va_end(ap);
	return reglist_to_bool(regList);
}
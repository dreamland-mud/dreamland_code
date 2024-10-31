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

bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...) 
{
	va_list ap;    
	va_start(ap, fmt);

	auto regList = behavior_trigger_with_result(obj->pIndexData->behaviors, trigType, fmt, ap);

	va_end(ap);
	return reglist_to_bool(regList);
}
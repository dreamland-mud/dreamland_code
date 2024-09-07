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


static bool behavior_trigger(GlobalBitvector &behaviors, const DLString &trigType, const char *fmt, va_list ap)
{
    bool rc = false;

	if (behaviors.empty())
		return rc;

	try {
		RegisterList trigArgs;

		// Collect all arguments
		WrapperBase::triggerArgs(trigArgs, fmt, ap);

		// Execute onXXX, postXXX triggers fro all behaviors associated with this item.
		for (int &bhvIndex: behaviors.toArray()) {
			Behavior *bhv = behaviorManager->find(bhvIndex);
			WrapperBase *bhvWrapper = bhv->getWrapper();
			
			// TODO: handle multiple onUse and Behavior::cmd.
			if (bhvWrapper && fenia_trigger(trigType, trigArgs, bhvWrapper, 0))
				rc = true;
		}

	} catch (const Scripting::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
		DLString methodId = "behavior on" + trigType;
        FeniaManager::getThis()->croak(0, Scripting::Register(methodId), e);
        return false;
    }
    return rc;
}

	
bool behavior_trigger(Room *room, const DLString &trigType, const char *fmt, ...) 
{
	va_list ap;    
	va_start(ap, fmt);
	bool rc = behavior_trigger(room->pIndexData->behaviors, trigType, fmt, ap);
	va_end(ap);
	return rc;
}

bool behavior_trigger(Character *ch, const DLString &trigType, const char *fmt, ...) 
{
	if (!ch->is_npc())
		return false;

	va_list ap;    
	va_start(ap, fmt);
	bool rc = behavior_trigger(ch->getNPC()->pIndexData->behaviors, trigType, fmt, ap);
	va_end(ap);
	return rc;
}

bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...) 
{
	va_list ap;    
	va_start(ap, fmt);
	bool rc = behavior_trigger(obj->pIndexData->behaviors, trigType, fmt, ap);
	va_end(ap);
	return rc;
}
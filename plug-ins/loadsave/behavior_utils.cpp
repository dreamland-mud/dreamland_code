#include "behavior_utils.h"
#include "fenia_utils.h"
#include "behavior.h"
#include "lex.h"
#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "object.h"
#include "character.h"
#include "room.h"

bool behavior_trigger(Object *obj, const DLString &trigType, const char *fmt, ...) 
{
    bool rc = false;

	if (obj->pIndexData->behaviors.empty())
		return rc;

    RegisterList trigArgs;

    // Collect all arguments
    {
	    va_list ap;    
    	va_start(ap, fmt);
    	WrapperBase::triggerArgs(trigArgs, fmt, ap);
    	va_end(ap);
    }

    // Execute onXXX, postXXX triggers fro all behaviors associated with this item.
	for (int &bhvIndex: obj->pIndexData->behaviors.toArray()) {
		Behavior *bhv = behaviorManager->find(bhvIndex);
		WrapperBase *bhvWrapper = bhv->getWrapper();
		
		// TODO: handle multiple onUse and Behavior::cmd.
		if (fenia_trigger(trigType, trigArgs, bhvWrapper, 0))
            rc = true;
	}

    return rc;
}

	

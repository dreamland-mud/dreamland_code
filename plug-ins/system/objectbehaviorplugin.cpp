/* $Id: objectbehaviorplugin.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "objectbehaviorplugin.h"
#include "objectbehavior.h"
#include "object.h"
#include "dreamland.h"

void ObjectBehaviorPlugin::initialization( ) {
    Object *obj;

    for (obj = object_list; obj; obj = obj->next) {
	if (!obj->behavior)
	    continue;

	if (obj->behavior->getType( ) == getName( )) {
	    obj->behavior.recover( );
	    obj->behavior->setObj( obj );	
	}
    }
    
}

void ObjectBehaviorPlugin::destruction( ) {
    Object *obj;

    /* XXX */
    if (dreamland->isShutdown( ))
	return;

    for (obj = object_list; obj; obj = obj->next) {
	if (!obj->behavior) 
	    continue;

	if (obj->behavior->getType( ) == getName( )) {
	    obj->behavior->unsetObj( );	
	    obj->behavior.backup( );
	}
    }
}

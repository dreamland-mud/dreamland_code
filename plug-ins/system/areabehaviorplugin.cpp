/* $Id: areabehaviorplugin.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "areabehaviorplugin.h"
#include "character.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

void AreaBehaviorPlugin::initialization( ) {
    AREA_DATA *area;

    for (area = area_first; area; area = area->next) {
        if (!area->behavior)
            continue;
        
        if (area->behavior->getType( ) == getName( )) {
            area->behavior.recover( );
            area->behavior->setArea( area );        
        }
    }
}

void AreaBehaviorPlugin::destruction( ) {
    AREA_DATA *area;

    /* XXX */
    if (dreamland->isShutdown( ))
        return;

    for (area = area_first; area; area = area->next) {
        if (!area->behavior)
            continue;
        
        if (area->behavior->getType( ) == getName( )) {
            area->behavior->unsetArea( );
            area->behavior.backup( );
        }
    }
}

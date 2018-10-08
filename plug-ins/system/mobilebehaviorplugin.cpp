/* $Id: mobilebehaviorplugin.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "mobilebehaviorplugin.h"
#include "mobilebehavior.h"
#include "npcharacter.h"
#include "dreamland.h"

void MobileBehaviorPlugin::initialization( ) 
{
    NPCharacter *mob;
    Character *wch;

    for (wch = char_list; wch; wch = wch->next) {
        mob = wch->getNPC( );

        if (!mob || !mob->behavior)
            continue;

        if (mob->behavior->getType( ) == getName( )) {
            mob->behavior.recover( );
            mob->behavior->setChar( mob );        
        }
    }
    
}

void MobileBehaviorPlugin::destruction( ) 
{
    NPCharacter *mob;
    Character *wch;

    /* XXX */
    if (dreamland->isShutdown( ))
        return;

    for (wch = char_list; wch; wch = wch->next) {
        mob = wch->getNPC( );

        if (!mob || !mob->behavior)
            continue;
        
        if (mob->behavior->getType( ) == getName( )) {
            mob->behavior->unsetChar( );        
            mob->behavior.backup( );
        }
    }
}

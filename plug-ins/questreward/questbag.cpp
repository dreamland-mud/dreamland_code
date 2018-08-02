/* $Id: questbag.cpp,v 1.1.2.8.18.1 2008/03/21 22:41:58 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */

#include "questbag.h"
#include "class.h"
#include "affect.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"

bool QuestBag::canLock( Character *ch ) 
{ 
    PCMemoryInterface *pcm;
    
    if (!obj->getOwner( ))
	return false;

    pcm = PCharacterManager::find( obj->getOwner( ) );

    if (!pcm)
	return false;

    if (pcm->getAttributes( ).isAvailable( "fullloot" ))
	return true;
    
    return obj->hasOwner( ch );
}


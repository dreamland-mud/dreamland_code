/* $Id: quest-impl.h,v 1.1.4.5.10.1 2007/09/29 19:33:59 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef __QUEST_IMPL_H__
#define __QUEST_IMPL_H__

#include "quest.h"

#include "npcharacter.h"
#include "object.h"

template<typename T>
inline bool Quest::check( Character *victim ) 
{
    T *bhv;
    
    if (!victim->is_npc( ) || !victim->getNPC()->behavior)
        return false;

    bhv = dynamic_cast<T *>( *victim->getNPC()->behavior );
    return (bhv && bhv->getHeroName( ) == charName.getValue( ));
}

template<typename T>
inline bool Quest::check( Object *obj ) 
{
    T *bhv;
    
    if (!obj->behavior) 
        return false;
        
    bhv = dynamic_cast<T *>( *obj->behavior );
    return (bhv && bhv->getHeroName( ) == charName.getValue( ));
}

#endif 

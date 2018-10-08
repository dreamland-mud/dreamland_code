/* $Id: subr.h,v 1.1.2.3 2005/09/11 22:50:53 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __SUBR_H__
#define __SUBR_H__

#include "exceptions.h"
#include "register-impl.h"

template <typename WT>
WT *wrapper_cast(const Scripting::Register &reg)
{
    WT *t = dynamic_cast<WT *>(reg.toHandler().getPointer());
    
    if(!t)
        throw Scripting::InvalidCastException(typeid(WT).name( ),
                reg.toHandler()->getType( ));

    return t;
}

template <typename WT>
WT *wrapper_cast(Scripting::Object *obj)
{
    WT *t = dynamic_cast<WT *>(obj->getHandler().getPointer());
    
    if(!t)
        throw Scripting::InvalidCastException(typeid(WT).name( ), 
                 obj->getHandler()->getType( ));

    return t;
}


#endif /* __SUBR_H__ */

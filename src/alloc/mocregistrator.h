/* $Id: mocregistrator.h,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * ruffina, DreamLand, 2005
 */
#ifndef __MOCREGISTRATOR_H__
#define __MOCREGISTRATOR_H__

#include "plugin.h"
#include "class.h"

template<typename C>
class MocRegistrator : public virtual Plugin {
public:
    typedef ::Pointer< MocRegistrator<C> > Pointer;

    virtual void initialization( ) 
    {
        Class::regMoc<C>( );
    }
    virtual void destruction( ) 
    {
        Class::unregMoc<C>( );
    }
};

#endif

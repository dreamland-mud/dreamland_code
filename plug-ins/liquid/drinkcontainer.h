/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DRINKCONTAINER_H__
#define __DRINKCONTAINER_H__

#include "objectbehaviormanager.h"

class DrinkContainer : public BasicObjectBehavior {
public:
    typedef ::Pointer<DrinkContainer> Pointer;

    virtual void pourOut( Character *, int );    
    virtual void pourOut( Character *, Character *, int );    
    virtual void pour( Character *, Object *, int );
    virtual void fill( Character *, Object *, int );
    virtual void drink( Character *, int );
};


#endif

#ifndef CRAFTWEARLOC_H
#define CRAFTWEARLOC_H

#include "defaultwearlocation.h"

class CraftTattooWearloc : public DefaultWearlocation {
XML_OBJECT    
public:
    typedef ::Pointer<CraftTattooWearloc> Pointer;

    virtual int canWear( Character *ch, Object *obj, int flags );
    virtual bool canRemove( Character *ch, Object *obj, int flags );
};

#endif

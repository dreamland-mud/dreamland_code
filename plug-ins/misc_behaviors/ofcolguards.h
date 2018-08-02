/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __OFCOLGUARDS_H__
#define __OFCOLGUARDS_H__

#include "wanderer.h"
#include "basicmobilebehavior.h"

class OfcolGuard : public Wanderer, public BasicMobileDestiny {
XML_OBJECT    
friend class OfcolMarshal;
public:
    typedef ::Pointer<OfcolGuard> Pointer;

    virtual void fight( Character * );
};

class OfcolMarshal: public BasicMobileDestiny {
XML_OBJECT    
public:
    typedef ::Pointer<OfcolMarshal> Pointer;

    virtual void fight( Character * );
};

#endif

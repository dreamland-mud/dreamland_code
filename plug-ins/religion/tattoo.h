/* $Id$
 *
 * ruffina, 2004
 */
#ifndef RELIGIONTATTOO_H
#define RELIGIONTATTOO_H

#include "objectbehavior.h"

class ReligionTattoo : public ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<ReligionTattoo> Pointer;
    
    virtual void fight( Character * );
};


#endif

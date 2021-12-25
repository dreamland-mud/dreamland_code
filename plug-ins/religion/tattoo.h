/* $Id$
 *
 * ruffina, 2004
 */
#ifndef RELIGIONTATTOO_H
#define RELIGIONTATTOO_H

#include "objectbehaviormanager.h"

class ReligionTattoo : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<ReligionTattoo> Pointer;
    
    virtual void fight( Character * );
};


#endif

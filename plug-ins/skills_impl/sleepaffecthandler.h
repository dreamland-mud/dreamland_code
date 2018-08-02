/* $Id$
 *
 * ruffina, 2004
 */
#ifndef SLEEPAFFECTHANDLER_H
#define SLEEPAFFECTHANDLER_H

#include "defaultaffecthandler.h"

class SleepAffectHandler : public virtual DefaultAffectHandler {
XML_OBJECT
public:
    typedef ::Pointer<SleepAffectHandler> Pointer;

    virtual void remove( Character * ); 
};

#endif

/* $Id: remortnanny.h,v 1.1.2.8.4.2 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef REMORTNANNY_H
#define REMORTNANNY_H

#include "descriptorstatelistener.h"

class PCharacter;
class Remorts;

class RemortNanny : public DescriptorStateListener {
public:
    typedef ::Pointer<RemortNanny> Pointer;

    virtual void run( int, int, Descriptor * );

};

#endif

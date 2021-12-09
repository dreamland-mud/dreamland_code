/* $Id: midgaardfountain.h,v 1.1.2.4 2005/08/26 13:08:19 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef MIDGAARDFOUNTAIN_H
#define MIDGAARDFOUNTAIN_H

#include "objectbehaviormanager.h"

class MidgaardFountain : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<MidgaardFountain> Pointer;

        virtual bool area( );
};

#endif


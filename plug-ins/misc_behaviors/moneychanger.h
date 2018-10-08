/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MONEYCHANGER_H
#define MONEYCHANGER_H

#include "basicmobilebehavior.h"

class MoneyChanger : public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<MoneyChanger> Pointer;
    
        virtual void bribe( Character *, int, int );
};

#endif


/* $Id: templeman.h,v 1.1.2.1 2007/09/10 23:46:50 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef TEMPLEMAN_H
#define TEMPLEMAN_H

#include "basicmobilebehavior.h"

class Templeman : public BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<Templeman> Pointer;
    
    Templeman( );

    virtual void speech( Character *, const char * );
    virtual void greet( Character * );
};

#endif


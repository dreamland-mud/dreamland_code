/* $Id: class_vampire.h,v 1.1.4.2 2005/11/26 16:59:51 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CLASS_VAMPIRE_H 
#define CLASS_VAMPIRE_H 

#include "basicmobilebehavior.h"

class VampireGuildmaster : public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<VampireGuildmaster> Pointer;
        
        virtual bool social( Character *, Character *, const DLString & );
};

#endif


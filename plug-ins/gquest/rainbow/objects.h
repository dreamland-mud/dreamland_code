/* $Id: objects.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef RAINBOW_OBJECTS_H
#define RAINBOW_OBJECTS_H

#include "gqobjects.h"

class RainbowPiece: public GlobalQuestObject {
XML_OBJECT    
public:
        typedef ::Pointer<RainbowPiece> Pointer;
        
        virtual void get( Character * );
        
        void config( int );
        
        XML_VARIABLE XMLInteger number;
};

#endif


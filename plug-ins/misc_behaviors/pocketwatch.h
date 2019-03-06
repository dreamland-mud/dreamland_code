/* $Id: pocketwatch.h,v 1.1.2.3 2005/08/26 13:08:19 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef POCKETWATCH_H
#define POCKETWATCH_H

#include "xmlboolean.h"
#include "xmlinteger.h"

#include "objectbehavior.h"

class PocketWatch : public ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<PocketWatch> Pointer;
    
        PocketWatch( );
        virtual bool prompt( Character *ch, char letter, ostringstream &buf );
        
        XML_VARIABLE XMLBoolean broken;
        XML_VARIABLE XMLInteger prevHour;
};

#endif


/* $Id: objects.h,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef INVASIONOBJECTS_H
#define INVASIONOBJECTS_H

#include "objectbehavior.h"
#include "xmlinteger.h"

class InvasionObj : public ObjectBehavior {
XML_OBJECT
friend class InvasionInstrument;
public:
        typedef ::Pointer<InvasionObj> Pointer;
    
        InvasionObj( );
        
        virtual void greet( Character * );

protected:
        virtual void actDestroy( Character * );
};

class InvasionInstrument : public ObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<InvasionInstrument> Pointer;
    
        InvasionInstrument( );
        
        virtual void wear( Character * );                 
        virtual bool use( Character *, const char * );
        
protected:
        virtual void actUse( Character *, Object * );
        virtual void actDestroy( Character * );

        XML_VARIABLE XMLInteger charges;
};

#endif


/* $Id: areabehavior.h,v 1.1.2.1.24.1 2007/06/26 07:24:24 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef AREABEHAVIOR_H
#define AREABEHAVIOR_H

#include "xmlvariablecontainer.h"
#include "xmlpersistent.h"

struct AreaIndexData;

class AreaBehavior : public XMLVariableContainer {
XML_OBJECT
public:    
    typedef ::Pointer<AreaBehavior> Pointer;
    
    AreaBehavior( );
    virtual ~AreaBehavior( );

    virtual void setArea( AreaIndexData * );
    virtual void unsetArea( );
    AreaIndexData * getArea( );

    inline virtual void update( ) { }
    
    static const DLString NODE_NAME;

protected:
    AreaIndexData *area;
};

extern template class XMLStub<AreaBehavior>;

#endif


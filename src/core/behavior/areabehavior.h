/* $Id: areabehavior.h,v 1.1.2.1.24.1 2007/06/26 07:24:24 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef AREABEHAVIOR_H
#define AREABEHAVIOR_H

#include "xmlvariablecontainer.h"
#include "xmlpersistent.h"

struct area_data;

class AreaBehavior : public XMLVariableContainer {
XML_OBJECT
public:    
    typedef ::Pointer<AreaBehavior> Pointer;
    
    AreaBehavior( );
    virtual ~AreaBehavior( );

    virtual void setArea( area_data * );
    virtual void unsetArea( );
    area_data * getArea( );

    inline virtual void update( ) { }
    
    static const DLString NODE_NAME;

protected:
    area_data *area;
};

extern template class XMLStub<AreaBehavior>;

#endif


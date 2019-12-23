/* $Id: xmlcontainer.h,v 1.6.2.2.18.1 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef __XMLCONTAINER_H__
#define __XMLCONTAINER_H__

#include "xmlvariable.h"

class XMLContainer : public virtual XMLVariable {
public:

    virtual void fromXML( const XMLNode::Pointer& ) ;
    virtual bool nodeFromXML( const XMLNode::Pointer & ) = 0;
};

#endif

/* $Id: xmlglobalreference.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 */
#ifndef __XMLGLOBALREFERENCE_H__
#define __XMLGLOBALREFERENCE_H__

#include "globalreference.h"
#include "xmlvariable.h"

class XMLGlobalReference : public virtual GlobalReferenceBase, 
                           public XMLVariable 
{
public:
    
    virtual void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& ) const;
};


#define XMLGLOBALREF_DECL(Type) \
    struct XML##Type##Reference : public Type##Reference, public XMLGlobalReference { \
	XML##Type##Reference( ); \
	virtual ~XML##Type##Reference( ); \
	inline XML##Type##Reference & operator = ( int index ) { \
	    assign( index ); \
	    return *this; \
	} \
    };

#define XMLGLOBALREF_IMPL(Type) \
    XML##Type##Reference::XML##Type##Reference( ) { } \
    XML##Type##Reference::~XML##Type##Reference( ) { }

#endif

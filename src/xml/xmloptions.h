/* $Id: xmloptions.h,v 1.1.2.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2009
 */
#ifndef XMLOPTIONS_H
#define XMLOPTIONS_H
// MOC_SKIP_BEGIN

#include "xmlboolean.h"
#include "xmlinteger.h"
#include "xmlstring.h"

template <typename ValType, typename ParentType>
class XMLOption : public ParentType {
public:	
    using ParentType::fromXML;
    using ParentType::getValue;
    
    XMLOption( ) : isOverriden( false )
    {
    }
    
    inline void override( ValType value ) {
	isOverriden = true;	
	setValue( value );
    }
    
    inline void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType ) {
	if (isOverriden)
	    return;

	ParentType::fromXML( node );
    }

private:
    bool isOverriden;
};


template <int def>
class XMLIntegerOption : public XMLOption<int, XMLIntegerNoDef<def> > {
public:
};

template <bool def>
class XMLBooleanOption : public XMLOption<bool, XMLBooleanNoDef<def> > {
public:

};

class XMLStringOption : public XMLStringNoEmpty {
public:
    void override( const char *value ) {
	isOverriden = true;
	assign( value );
    }
    
    inline void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType ) {
	if (isOverriden)
	    return;

	XMLStringNoEmpty::fromXML( node );
    }

private:
    bool isOverriden;
};

// MOC_SKIP_END
#endif

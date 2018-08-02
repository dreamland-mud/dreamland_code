/* $Id: xmlvariablecontainer.h,v 1.13.2.4.8.2 2009/10/11 18:35:39 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 *
 * naming conventions from XMLVariableContainer by NoFate, 2000
 */

#ifndef __XMLVARIABLECONTAINER_H__
#define __XMLVARIABLECONTAINER_H__

#include "xmlpolymorphvariable.h"
#include "xmlcontainer.h"

class XMLNode;

class XMLVariableContainer : public virtual XMLPolymorphVariable, 
                             public virtual XMLContainer 
{
protected:
    typedef ::Pointer<XMLVariableContainer> Pointer;

public:

    virtual const DLString & getType( ) const;
    virtual bool nodeFromXML( const XMLNode::Pointer & );
    virtual bool toXML( XMLNode::Pointer & ) const;

private:
    virtual bool mocNodeFromXML( const XMLNode::Pointer & ) = 0;
    virtual bool mocToXML( XMLNode::Pointer & ) const = 0;
    virtual const DLString & mocGetType( ) const = 0;
};

#define XML_OBJECT \
	class __MetaInfo__; \
    public: \
	static const DLString MOC_TYPE; \
    private: \
	virtual const DLString & mocGetType( ) const; \
	virtual bool mocNodeFromXML( const XMLNode::Pointer & ); \
	virtual bool mocToXML( XMLNode::Pointer & ) const; 

#define XML_VARIABLE
    
#endif

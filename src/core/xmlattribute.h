/* $Id: xmlattribute.h,v 1.3.2.10.6.1 2007/06/26 07:24:21 rufina Exp $
 *
 * ruffina, 2004
 * named after XMLAttribute by NoFate, 2001
 */

#ifndef XMLATTRIBUTE_H
#define XMLATTRIBUTE_H

#include "xmlpolymorphvariable.h"
#include "xmlpersistent.h"
#include "register-decl.h"

template <typename ArgType>
class AttributeEventHandler;

class XMLAttribute : public virtual XMLPolymorphVariable {
friend class XMLAttributes;
public:
    typedef ::Pointer<XMLAttribute> Pointer;
    
    virtual const DLString &getName( ) const;
    virtual Scripting::Register toRegister() const;

    template <typename ArgType>
    bool handleEvent( const ArgType &args )
    {
        AttributeEventHandler<ArgType> *handler;

        handler = dynamic_cast<AttributeEventHandler<ArgType> *>( this );
        
        if (handler)
            return handler->handle( args );
        else
            return false;
    }

protected:
    virtual void init( );
    virtual void destroy( );
};

extern template class XMLStub<XMLAttribute>;

template <typename ArgType>
class AttributeEventHandler : public virtual XMLAttribute {
public:
    virtual bool handle( const ArgType & ) { 
        return false;
    };
};


#endif

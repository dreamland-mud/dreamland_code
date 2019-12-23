/* $Id: xmlpcpredicates.h,v 1.1.2.1 2007/09/11 00:22:30 rufina Exp $
 *
 * ruffina, 2004
 */
 
#ifndef XMLPCPREDICATES_H
#define XMLPCPREDICATES_H

#include "xmlpredicate.h"
#include "xmlstring.h"

class PCharacter;

class XMLStringPredicate : public XMLPredicate, public XMLStringVariable {
public:
    
    virtual bool eval( DLObject * ) const;
    virtual void fromXML( const XMLNode::Pointer & ) ;
    virtual bool toXML( XMLNode::Pointer& parent ) const;

private:
    virtual DLString getString( DLObject * ) const = 0;
};

class XMLPCStringPredicate : public XMLStringPredicate {
private:
    virtual DLString getString( DLObject * ) const;
    virtual DLString getString( PCharacter * ) const = 0;
};

class XMLPCClassPredicate : public XMLPCStringPredicate {
public:
    typedef ::Pointer<XMLPCClassPredicate> Pointer;

    static const DLString TYPE;
    virtual const DLString & getType( ) const 
    {
        return TYPE;
    }

private:
    virtual DLString getString( PCharacter * ) const;
};

class XMLPCRacePredicate : public XMLPCStringPredicate {
public:
    typedef ::Pointer<XMLPCRacePredicate> Pointer;
    
    static const DLString TYPE;
    virtual const DLString & getType( ) const 
    {
        return TYPE;
    }

private:
    virtual DLString getString( PCharacter * ) const;
};

class XMLPCAlignPredicate : public XMLPCStringPredicate {
public:
    typedef ::Pointer<XMLPCAlignPredicate> Pointer;
    
    static const DLString TYPE;
    virtual const DLString & getType( ) const 
    {
        return TYPE;
    }

private:
    virtual DLString getString( PCharacter * ) const;
};

class XMLPCEthosPredicate : public XMLPCStringPredicate {
public:
    typedef ::Pointer<XMLPCEthosPredicate> Pointer;
    
    static const DLString TYPE;
    virtual const DLString & getType( ) const 
    {
        return TYPE;
    }

private:
    virtual DLString getString( PCharacter * ) const;
};

class XMLPCSexPredicate : public XMLPCStringPredicate {
public:
    typedef ::Pointer<XMLPCSexPredicate> Pointer;
    
    static const DLString TYPE;
    virtual const DLString & getType( ) const 
    {
        return TYPE;
    }

private:
    virtual DLString getString( PCharacter * ) const;
};

#endif

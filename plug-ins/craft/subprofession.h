#ifndef SUBPROFESSION_H
#define SUBPROFESSION_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmltableelement.h"

#include "helpmanager.h"
#include "markuphelparticle.h"


class SubProfession;

class SubProfessionHelp : public virtual XMLHelpArticle,
                       public virtual MarkupHelpArticle {
public:
    typedef ::Pointer<SubProfessionHelp> Pointer;

    virtual void setProfession( ::Pointer<SubProfession> );
    virtual void unsetProfession( );

    virtual void getRawText( Character *, ostringstream & ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    ::Pointer<SubProfession> prof;
};

inline const DLString & SubProfessionHelp::getType( ) const
{
    return TYPE;
}

class SubProfession : public XMLTableElement,
	              public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<SubProfession> Pointer;
    
    SubProfession( );
    virtual ~SubProfession( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    virtual void loaded( );
    virtual void unloaded( );
    
    inline virtual const DLString &getRusName( ) const;
    inline virtual const DLString &getMltName( ) const;
    inline virtual int getBaseExp( ) const;
    
protected:
    XML_VARIABLE XMLString  name, rusName, mltName;
    XML_VARIABLE XMLInteger baseExp;
    XML_VARIABLE XMLPointerNoEmpty<SubProfessionHelp> help;
};


inline const DLString & SubProfession::getName( ) const
{
    return name;
}

inline void SubProfession::setName( const DLString &name ) 
{
    this->name = name;
}

inline const DLString & SubProfession::getRusName( ) const
{
    return rusName;
}

inline const DLString &SubProfession::getMltName( ) const
{
    return mltName;
}

inline int SubProfession::getBaseExp( ) const
{
    return baseExp;
}

#endif

#ifndef SUBPROFESSION_H
#define SUBPROFESSION_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmltableelement.h"
#include "plugin.h"
#include "oneallocate.h"
#include "grammar_entities.h"
#include "craftattribute.h"

#include "helpmanager.h"
#include "markuphelparticle.h"

class PCharacter;
class CraftProfession;

class CraftProfessionHelp : public virtual XMLHelpArticle,
                       public virtual MarkupHelpArticle {
public:
    typedef ::Pointer<CraftProfessionHelp> Pointer;

    virtual void setProfession( ::Pointer<CraftProfession> );
    virtual void unsetProfession( );

    virtual void getRawText( Character *, ostringstream & ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    ::Pointer<CraftProfession> prof;
};

inline const DLString & CraftProfessionHelp::getType( ) const
{
    return TYPE;
}

class CraftProfession : public XMLTableElement,
                      public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<CraftProfession> Pointer;
    
    CraftProfession( );
    virtual ~CraftProfession( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual DLString getNameFor( Character *, const Grammar::Case & = Grammar::Case::NONE ) const;
    inline virtual const DLString &getRusName( ) const;
    inline virtual const DLString &getMltName( ) const;
    virtual void setLevel( PCharacter *, int level = -1 ) const;
    virtual int getLevel( PCharacter * ) const;

    virtual int getExpToLevel( PCharacter *, int level = -1 ) const;
    virtual int getExpThisLevel( PCharacter * ) const;
    virtual int getExpPerLevel( PCharacter *, int level = -1  ) const;
    virtual int getTotalExp( PCharacter * ) const;
    virtual int gainExp( PCharacter *ch, int xp ) const;
    
protected:
    XML_VARIABLE XMLString  name, rusName, mltName;
    XML_VARIABLE XMLInteger baseExp;
    XML_VARIABLE XMLInteger maxLevel;
    XML_VARIABLE XMLPointerNoEmpty<CraftProfessionHelp> help;
};


inline const DLString & CraftProfession::getName( ) const
{
    return name;
}

inline void CraftProfession::setName( const DLString &name ) 
{
    this->name = name;
}

inline const DLString & CraftProfession::getRusName( ) const
{
    return rusName;
}

inline const DLString &CraftProfession::getMltName( ) const
{
    return mltName;
}

class CraftProfessionManager : public OneAllocate, public virtual Plugin {
public:
    typedef ::Pointer<CraftProfessionManager> Pointer;
    typedef std::map<DLString, CraftProfession::Pointer> Professions;

    CraftProfessionManager( );
    virtual ~CraftProfessionManager( );
    
    virtual void initialization( );
    virtual void destruction( );
    void load( CraftProfession::Pointer );
    void unload( CraftProfession::Pointer );
    CraftProfession::Pointer get( const DLString & ) const;
    CraftProfession::Pointer lookup( const DLString & ) const;
    list<CraftProfession::Pointer> getProfessions() const;

protected:
    Professions profs;
};

extern CraftProfessionManager * craftProfessionManager;

#endif

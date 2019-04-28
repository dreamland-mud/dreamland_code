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

/**
 * Handles experience calculations for given profession and player.
 */
class ExperienceCalculator : public virtual DLObject {
public:
    typedef ::Pointer<ExperienceCalculator> Pointer;

    /**
     * Return total amount of experience you needed to gain from level 1 to reach this particular level.
     * If level is unspecified, current one is assumed.
     */
    virtual int expPerLevel(int level_ = -1) const; 

    /**
     * Return total amount of experience you need to gain at your current proficiency
     * level, in order to progress to the next one.
     */
    virtual int expThisLevel() const;

    /**
     * Return how much exp is left to gain to reach next level after this particular one.
     * If level is unspecified, current one is assumed.
     */
    virtual int expToLevel(int level_ = -1) const;

    /**
     * Return total amount of experience gained so far.
     */
    virtual int totalExp() const;
};

class CraftProfessionHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<CraftProfessionHelp> Pointer;

    virtual void setProfession( ::Pointer<CraftProfession> );
    virtual void unsetProfession( );

    virtual DLString getTitle(const DLString &label) const;
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
    inline virtual int getBaseExp( ) const;
    inline virtual int getMaxLevel( ) const;

    virtual ExperienceCalculator::Pointer getCalculator(PCharacter *ch) const;
    virtual void gainExp( PCharacter *ch, int xp ) const;
    
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

inline int CraftProfession::getBaseExp( ) const
{
    return baseExp;
}

inline int CraftProfession::getMaxLevel( ) const
{
    return maxLevel;
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

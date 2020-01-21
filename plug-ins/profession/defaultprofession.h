/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DEFAULTPROFESSION_H__
#define __DEFAULTPROFESSION_H__

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmltableelement.h"

#include "helpmanager.h"
#include "markuphelparticle.h"
#include "profession.h"

class DefaultProfession;

/*
 * ProfessionHelp
 */
class ProfessionHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<ProfessionHelp> Pointer;

    virtual void setProfession( ::Pointer<DefaultProfession> prof );
    virtual void unsetProfession( );

    virtual DLString getTitle(const DLString &label) const;
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void save() const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    ::Pointer<DefaultProfession> prof;
};

inline const DLString & ProfessionHelp::getType( ) const
{
    return TYPE;
}

/** An article that automatically displays all skills of a certain class. */
class ClassSkillHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<ClassSkillHelp> Pointer;    

    virtual void setProfession( ::Pointer<DefaultProfession> prof );
    virtual void unsetProfession( );

    virtual const DLString & getType( ) const;
    static const DLString TYPE;
    virtual DLString getTitle(const DLString &label) const;
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void save() const;

protected:
    ::Pointer<DefaultProfession> prof;
};
 
inline const DLString & ClassSkillHelp::getType( ) const
{
    return TYPE;
}

/*
 * ProfessionTitles
 */
class ProfessionTitles : public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<ProfessionTitles> Pointer;
    
    virtual const DLString & build( const PCMemoryInterface * ) const = 0;
};

/*
 * ProfessionTitlesByConstant
 */
class ProfessionTitlesByConstant : public ProfessionTitles, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ProfessionTitlesByConstant> Pointer;
    
    virtual const DLString & build( const PCMemoryInterface * ) const;

protected:
    XML_VARIABLE XMLString title;
};

/*
 * ProfessionTitlePair
 */
class ProfessionTitlePair : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ProfessionTitlePair> Pointer;
    
    XML_VARIABLE XMLString male;
    XML_VARIABLE XMLString female;
};

typedef XMLVectorBase<ProfessionTitlePair> ProfessionTitlePairVector;

/*
 * ProfessionTitlesByLevel
 */
class ProfessionTitlesByLevel : public ProfessionTitles, 
                                public ProfessionTitlePairVector
{
public:
    typedef ::Pointer<ProfessionTitlesByLevel> Pointer;
    
    virtual const DLString & build( const PCMemoryInterface * ) const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    static const DLString TYPE;
};


/*
 * DefaultProfession
 */
class DefaultProfession : public Profession, 
                          public XMLTableElement,
                          public XMLVariableContainer 
{
XML_OBJECT
friend class ProfessionHelp;
public:
    typedef ::Pointer<DefaultProfession> Pointer;
    
    DefaultProfession( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getRusName( ) const;
    virtual const DLString &getMltName( ) const;
    virtual DLString getNameFor( Character *, const Grammar::Case & = Grammar::Case::NONE ) const;
    virtual DLString getWhoNameFor( Character * ) const;
    virtual int  getWeapon( ) const;
    virtual int  getSkillAdept( ) const;
    virtual int  getParentAdept( ) const;
    virtual int  getThac00( Character * = NULL ) const;
    virtual int  getThac32( Character * = NULL ) const;
    virtual int  getHpRate( ) const;
    virtual int  getManaRate( ) const;
    virtual int  getPoints( ) const;
    virtual int getStat( bitnumber_t, Character * = NULL ) const;
    virtual int  getWearModifier( int ) const;
    virtual const DLString & getTitle( const PCMemoryInterface * ) const;
    virtual GlobalBitvector toVector( Character * = NULL ) const;
    virtual Flags getFlags( Character * = NULL ) const;
    
    virtual bool isPlayed( ) const;
    virtual const Flags & getSex( ) const;
    virtual const Flags & getEthos( ) const;
    virtual const Flags & getAlign( ) const;

    virtual int getMinAlign( ) const;
    virtual int getMaxAlign( ) const;

protected:
    XML_VARIABLE XMLString  whoName, whoNameRus, rusName, mltName;
    XML_VARIABLE XMLInteger weapon;
    XML_VARIABLE XMLInteger skillAdept;
    XML_VARIABLE XMLIntegerNoEmpty parentAdept;
    XML_VARIABLE XMLInteger thac00;
    XML_VARIABLE XMLInteger thac32;
    XML_VARIABLE XMLInteger hpRate;
    XML_VARIABLE XMLInteger manaRate;
    XML_VARIABLE XMLInteger points;
    XML_VARIABLE XMLEnumerationArray stats;
    XML_VARIABLE XMLEnumerationArray wearModifiers;
    XML_VARIABLE XMLPointer<ProfessionTitles> titles;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    
    XML_VARIABLE XMLFlags align;
    XML_VARIABLE XMLInteger minAlign, maxAlign;
    XML_VARIABLE XMLFlags ethos;
    XML_VARIABLE XMLFlags sex;
    XML_VARIABLE XMLPointerNoEmpty<ProfessionHelp> help;
    XML_VARIABLE XMLPointerNoEmpty<ClassSkillHelp> skillHelp;
};

inline const DLString & DefaultProfession::getName( ) const
{
    return Profession::getName( );
}

inline void DefaultProfession::setName( const DLString &name ) 
{
    this->name = name;
}

inline bool DefaultProfession::isValid( ) const
{
    return true;
}

#endif

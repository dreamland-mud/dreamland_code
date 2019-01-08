/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTRELIGION_H
#define DEFAULTRELIGION_H

#include "xmlstring.h"
#include "xmlflags.h"
#include "xmltableelement.h"
#include "xmlglobalbitvector.h"
#include "xmlvariablecontainer.h"
#include "xmlenumeration.h"
#include "xmlboolean.h"

#include "religion.h"
#include "helpmanager.h"
#include "markuphelparticle.h"

class Skill;
class Liquid;

/*
 * ReligionHelp
 */
class ReligionHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<ReligionHelp> Pointer;

    virtual void setReligion( Religion::Pointer );
    virtual void unsetReligion( );

    virtual void getRawText( Character *, ostringstream & ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    Religion::Pointer religion;
};

inline const DLString & ReligionHelp::getType( ) const
{
    return TYPE;
}

class GodLikes : public XMLVariableContainer {
XML_OBJECT
public:
    GodLikes();

    XML_VARIABLE XMLGlobalBitvector skills;
    XML_VARIABLE XMLGlobalBitvector skillGroups;
    XML_VARIABLE XMLFlags           items;
    XML_VARIABLE XMLGlobalBitvector liquids;
    XML_VARIABLE XMLFlags           liquidFlags;
    XML_VARIABLE XMLBooleanNoFalse  books;
};

class DefaultReligion : public Religion, public XMLTableElement, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<DefaultReligion> Pointer;
    
    DefaultReligion( );
    
    virtual const DLString & getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual void setName( const DLString & );
    virtual bool isValid( ) const;
    virtual int getSex() const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getShortDescr( ) const;
    virtual const DLString &getDescription( ) const;
    virtual bool isAllowed( Character * ) const;
    virtual const DLString& getNameFor( Character * ) const;
    
    inline const Flags & getAlign() const;
    inline const Flags & getEthos() const;
    
    virtual bool likesSpell(Skill *) const;
    virtual bool likesDrink(const Liquid *) const;
    virtual bool likesItem(Object *) const;
    virtual bool likesBook(Object *) const;

protected:
    XML_VARIABLE XMLString  shortDescr;
    XML_VARIABLE XMLString  nameRus;
    XML_VARIABLE XMLString  description;
    XML_VARIABLE XMLFlags   align, ethos;
    XML_VARIABLE XMLGlobalBitvector  races;
    XML_VARIABLE XMLPointerNoEmpty<ReligionHelp> help;
    XML_VARIABLE XMLEnumeration sex;
    XML_VARIABLE GodLikes likes;
};

inline const Flags & DefaultReligion::getAlign() const
{
    return align;
}

inline const Flags & DefaultReligion::getEthos() const
{
    return ethos;
}

#endif

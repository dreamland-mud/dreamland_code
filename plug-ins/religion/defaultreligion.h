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
#include "xmltableloaderplugin.h"
#include "xmlglobalarray.h"
#include "xmlinteger.h"

#include "religion.h"
#include "helpmanager.h"
#include "markuphelparticle.h"

class Skill;
class Liquid;
class DefaultReligion;

TABLE_LOADER_DECL(ReligionLoader);

/*
 * ReligionHelp
 */
class ReligionHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<ReligionHelp> Pointer;

    virtual void setReligion( ::Pointer<DefaultReligion> religion );
    virtual void unsetReligion( );
    virtual void save() const;

    virtual DLString getTitle(const DLString &label) const;
    virtual void getRawText( Character *, ostringstream & ) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    ::Pointer<DefaultReligion> religion;
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
    XML_VARIABLE XMLBooleanNoFalse  stolen;
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
    virtual bool available(Character *) const;
    DLString reasonWhy(Character *) const;
    virtual const DLString& getNameFor( Character * ) const;
    
    inline const Flags & getAlign() const;
    inline const Flags & getEthos() const;
    
    virtual bool likesSpell(Skill *) const;
    virtual bool likesDrink(const Liquid *) const;
    virtual bool likesItem(Object *) const;
    virtual bool likesBook(Object *) const;
    virtual bool likesStolen(Object *) const;
    virtual bool ignoresItem(Object *) const;

// Fields are public to simplify online editing.
    XML_VARIABLE XMLString  shortDescr;
    XML_VARIABLE XMLString  nameRus, nameRusFemale;
    XML_VARIABLE XMLString  description;
    XML_VARIABLE XMLFlags   align, ethos;
    XML_VARIABLE XMLGlobalBitvector  races;
    XML_VARIABLE XMLGlobalBitvector  classes;
    XML_VARIABLE XMLPointerNoEmpty<ReligionHelp> help;
    XML_VARIABLE XMLEnumeration sex;
    XML_VARIABLE GodLikes likes;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLEnumerationArray minstat, maxstat;
    XML_VARIABLE XMLGlobalBitvector clans;
    XML_VARIABLE XMLIntegerNoEmpty minage, maxage;
    XML_VARIABLE XMLIntegerNoEmpty tattooVnum;
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

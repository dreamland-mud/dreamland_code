/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTSKILLGROUP_H
#define DEFAULTSKILLGROUP_H

#include "xmlvariablecontainer.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmltableelement.h"
#include "xmlinteger.h"
#include "xmlrussianstring.h"

#include "skillgrouphelp.h"
#include "skillgroup.h"

class Skill;

class DefaultSkillGroup : public SkillGroup, public XMLTableElement, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultSkillGroup> Pointer;
    
    DefaultSkillGroup( );
    
    inline virtual const DLString & getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual bool matchesUnstrict( const DLString & ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getRussianName( ) const;
    virtual const DLString &getShortDescr( ) const;
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual void show( PCharacter *, ostringstream & ) const;
    virtual int getPracticer( ) const;

protected:
    virtual void listSkills( PCharacter *, ostringstream & ) const;
    virtual void listPracticers( PCharacter *, ostringstream & ) const;
    virtual char getSkillColor( Skill *, PCharacter * ) const;

    XML_VARIABLE XMLStringNoEmpty    shortDescr;
    XML_VARIABLE XMLRussianString    nameRus;
    XML_VARIABLE XMLBoolean          nopet;
    XML_VARIABLE XMLBoolean          hidden;
    XML_VARIABLE XMLBoolean          autoHelp;
    XML_VARIABLE XMLPointerNoEmpty<SkillGroupHelp> help;
    XML_VARIABLE XMLIntegerNoEmpty   practicer;
};

inline const DLString & DefaultSkillGroup::getName( ) const
{
    return SkillGroup::getName( );
}

inline void DefaultSkillGroup::setName( const DLString &name ) 
{
    this->name = name;
}

inline bool DefaultSkillGroup::isValid( ) const
{
    return true;
}

#endif

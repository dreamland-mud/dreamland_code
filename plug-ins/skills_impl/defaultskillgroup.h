/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTSKILLGROUP_H
#define DEFAULTSKILLGROUP_H

#include "xmlvariablecontainer.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlstringlist.h"
#include "xmltableelement.h"
#include "xmlinteger.h"
#include "xmlrussianstring.h"
#include "xmlmultistring.h"
#include "xmlglobalbitvector.h"
#include "skillgrouphelp.h"
#include "skillgroup.h"

class Skill;

class DefaultSkillGroup : public SkillGroup, public XMLTableElement, public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultSkillGroup> Pointer;
    
    DefaultSkillGroup( );
    
    virtual const DLString & getName( ) const;
    virtual void setName( const DLString & );
    inline virtual bool isValid( ) const;
    virtual bool matchesUnstrict( const DLString & ) const;
    virtual void loaded( );
    virtual void unloaded( );
    
    virtual const DLString &getRussianName( ) const;
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual void show( PCharacter *, ostringstream & ) const;
    virtual int getPracticer( ) const;

    XML_VARIABLE XMLMultiString name;
    XML_VARIABLE XMLBoolean          hidden;
    XML_VARIABLE XMLBoolean          autoHelp;
    XML_VARIABLE XMLPointer<SkillGroupHelp> help;
    XML_VARIABLE XMLIntegerNoEmpty   practicer;
    XML_VARIABLE XMLStringList msgRoom, msgSelf, msgVict;
    XML_VARIABLE XMLStringNoEmpty path;
    XML_VARIABLE XMLGlobalBitvector gods;

protected:
    virtual void listSkills( PCharacter *, ostringstream & ) const;
    virtual void listPracticers( PCharacter *, ostringstream & ) const;
    virtual char getSkillColor( Skill *, PCharacter * ) const;
};


inline bool DefaultSkillGroup::isValid( ) const
{
    return true;
}

#endif

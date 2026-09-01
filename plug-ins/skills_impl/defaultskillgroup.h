/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTSKILLGROUP_H
#define DEFAULTSKILLGROUP_H

#include "lang.h"
#include "xmlvariablecontainer.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlstringlist.h"
#include "xmltableelement.h"
#include "xmlinteger.h"
#include "xmlinflectedstring.h"
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
    virtual bool matchesStrict( const DLString &str ) const;
    virtual bool matchesUnstrict( const DLString &str ) const;
    virtual bool matchesSubstring( const DLString &str ) const;
    virtual const DLString& getNameFor( Character * ) const;
    virtual const DLString& getNameFor( lang_t lang ) const;
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
    // Alignment-tiered cast flavour: optional dark / neutral gesture sets. When a
    // group defines them, an evil / neutral caster gets these instead of the base
    // (light) lines -- see DefaultSpell::collectFlavours. Empty = fall back to base.
    XML_VARIABLE XMLStringList msgRoomEvil, msgSelfEvil, msgVictEvil;
    XML_VARIABLE XMLStringList msgRoomNeutral, msgSelfNeutral, msgVictNeutral;
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

/* $Id$
 *
 * ruffina, 2004
 */
#ifndef SKILLGROUP_H
#define SKILLGROUP_H

#include <sstream>
#include "oneallocate.h"
#include "globalregistryelement.h"
#include "globalregistry.h"
#include "globalreference.h"
#include "xmlglobalreference.h"
#include "bitstring.h"

#define GROUP( name ) static SkillGroupReference group_##name( #name )

class Flags;
class GlobalArray;
class Character;
class PCharacter;

/*
 * SkillGroup
 */
class SkillGroup : public GlobalRegistryElement {
public:
    typedef ::Pointer<SkillGroup> Pointer;
    
    SkillGroup( );
    SkillGroup( const DLString & );
    virtual ~SkillGroup( );

    virtual bool isValid( ) const;
    virtual const DLString &getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual const DLString &getShortDescr( ) const;
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual void show( PCharacter *, ostringstream & ) const;
    virtual int getPracticer( ) const;

protected:
    DLString name;
};
    

/*
 * SkillGroupManager
 */
class SkillGroupManager : public GlobalRegistry<SkillGroup>, public OneAllocate {
public:
    SkillGroupManager( );
    virtual ~SkillGroupManager( );
    
    inline static SkillGroupManager *getThis( );
private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

extern SkillGroupManager * skillGroupManager;

inline SkillGroupManager * SkillGroupManager::getThis( )
{   
    return skillGroupManager;
}

/*
 * SkillGroupAction
 */
class SkillGroupAction {
public:
    typedef SkillGroup::Pointer SkillGroupPointer;
    
    virtual ~SkillGroupAction( );

    virtual void setSkillGroup( SkillGroupPointer ) = 0;
    virtual void unsetSkillGroup( ) = 0;
    virtual SkillGroupPointer getSkillGroup( ) const = 0;
};


GLOBALREF_DECL(SkillGroup)
XMLGLOBALREF_DECL(SkillGroup)

#endif

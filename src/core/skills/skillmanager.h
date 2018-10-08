/* $Id: skillmanager.h,v 1.1.2.4.6.1 2008/05/19 16:38:39 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef SKILLMANAGER_H
#define SKILLMANAGER_H

#include "oneallocate.h"
#include "globalregistry.h"

class Skill;
class Character;
class SkillManager;

extern SkillManager *skillManager;

class SkillManager : public GlobalRegistry<Skill>, public OneAllocate {
public:        
    typedef ::Pointer<SkillManager> Pointer;
    
    SkillManager( );
    virtual ~SkillManager( );
    
    int unstrictLookup( const DLString &, Character * ch = NULL ) const;

    static inline SkillManager* getThis( )        
    {
        return skillManager;
    }
    
private:
    virtual GlobalRegistryElement::Pointer getDumbElement( const DLString & ) const;
};

#endif

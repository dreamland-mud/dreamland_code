/* $Id: wrappermanager.h,v 1.1.2.3.18.2 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __WRAPPERMANAGER_H__
#define __WRAPPERMANAGER_H__

#include "plugin.h"
#include "register-decl.h"
#include "feniamanager.h"

#define ROOM_VNUM2ID(v) (((v) << 4) | 1)
#define OBJ_VNUM2ID(v) (((v) << 4) | 2)
#define MOB_VNUM2ID(v) (((v) << 4) | 3)
#define AREA_VNUM2ID(v) (((v) << 4) | 4)

class WrapperManager: public WrapperManagerBase, public Plugin {
public:
    
    virtual Scripting::Register getWrapper( Character * );
    virtual Scripting::Register getWrapper( ::Object * );
    virtual Scripting::Register getWrapper( Room * );
    virtual Scripting::Register getWrapper( mob_index_data * );
    virtual Scripting::Register getWrapper( obj_index_data * );
    virtual Scripting::Register getWrapper( AreaIndexData * );
    virtual Scripting::Register getWrapper( Spell * );
    virtual Scripting::Register getWrapper( AffectHandler * );
    virtual Scripting::Register getWrapper( Affect * );
    virtual Scripting::Register getWrapper( SkillCommand * );
    virtual Scripting::Register getWrapper( WrappedCommand * );
    virtual Scripting::Register getWrapper( AreaQuest * );
    virtual Scripting::Register getWrapper( Behavior * );
    
    virtual void linkWrapper( Character * );
    virtual void linkWrapper( ::Object * );
    virtual void linkWrapper( Room * );
    virtual void linkWrapper( mob_index_data * );
    virtual void linkWrapper( obj_index_data * );
    virtual void linkWrapper( AreaIndexData * );
    virtual void linkWrapper( Spell * );
    virtual void linkWrapper( AffectHandler * );
    virtual void linkWrapper( Affect * );
    virtual void linkWrapper( SkillCommand * );
    virtual void linkWrapper( WrappedCommand * );
    virtual void linkWrapper( AreaQuest * );
    virtual void linkWrapper( Behavior * );

    virtual void getTarget( const Scripting::Register &, Character *& );
    
    virtual void initialization( );
    virtual void destruction( );

    static WrapperManager* getThis( );

private:
    template <typename WrapperType, typename TargetType>
    Scripting::Register wrapperAux( long long, TargetType );
    
    template <typename WrapperType, typename TargetType>
    void linkAux( long long, TargetType );
};


#endif


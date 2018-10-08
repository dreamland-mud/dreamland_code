/* $Id: skilleventhandlertemplate.h,v 1.1.2.1 2010-09-05 13:57:11 rufina Exp $
 *
 * ruffina, 2010
 */
#ifndef        __SKILLEVENTHANDLERTEMPLATE_H__
#define __SKILLEVENTHANDLERTEMPLATE_H__

// MOC_SKIP_BEGIN
#include "skilleventhandler.h"
#include "plugininitializer.h"
#include "classselfregistratorplugin.h"
#include "pcharacter.h"
#include "object.h"

template <const char *&tn>
struct SkillEventHandlerTemplate : public SkillEventHandler, public ClassSelfRegistratorPlugin<tn> {
    typedef ::Pointer<SkillEventHandlerTemplate> Pointer;

    virtual bool putItem( va_list args ) { 
        Character *ch = va_arg( args, Character * );
        Object *item = va_arg( args, Object * );
        Object *container = va_arg( args, Object * );
        return put( ch, item, container ); 
    }
    virtual bool useItem( va_list args ) { 
        Character *ch = va_arg( args, Character * );
        Object *item = va_arg( args, Object * );
        const char *arg = va_arg( args, char * );
        return use( ch, item, arg ); 
    }
    virtual bool dropItem( va_list args ) { 
        Character *ch = va_arg( args, Character * );
        Object *item = va_arg( args, Object * );
        return drop( ch, item ); 
    }
    virtual bool fetchItem( va_list args ) { 
        Character *ch = va_arg( args, Character * );
        Object *item = va_arg( args, Object * );
        Object *container = va_arg( args, Object * );
        return fetch( ch, item, container ); 
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }

protected:
    virtual bool put( Character *, Object *, Object * ) { 
        return false; 
    }
    virtual bool use( Character *, Object *, const char * ) { 
        return false; 
    }
    virtual bool drop( Character *, Object * ) { 
        return false; 
    }
    virtual bool fetch( Character *, Object *, Object * ) { 
        return false; 
    }
};

#define SKILLEVENT_DUMMY(x)         dummySkillEvent_ ##x## _TypeName
#define SKILLEVENT(x) SkillEventHandlerTemplate<SKILLEVENT_DUMMY(x)>

#define SKILLEVENT_DECL(x) \
const char *SKILLEVENT_DUMMY(x) = "SKILLEVENT(" #x ")"; \
PluginInitializer<SKILLEVENT(x)> dummySkillEvent_ ##x## _init;

#define SKILLEVENTHANDLER(x) template <> bool SKILLEVENT(x)

// MOC_SKIP_END
#endif

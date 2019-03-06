/* $Id: affecthandlertemplate.h,v 1.1.2.7 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef        __AFFECTHANDLERTEMPLATE_H__
#define __AFFECTHANDLERTEMPLATE_H__

// MOC_SKIP_BEGIN
#include "defaultaffecthandler.h"
#include "plugininitializer.h"
#include "classselfregistratorplugin.h"

template <const char *&tn>
struct AffectHandlerTemplate : public DefaultAffectHandler, public ClassSelfRegistratorPlugin<tn> {
    typedef ::Pointer<AffectHandlerTemplate> Pointer;

    virtual void toStream( ostringstream &buf, Affect *af ) 
    {
        DefaultAffectHandler::toStream( buf, af );
    }
    virtual void remove( Character * ch ) 
    { 
        DefaultAffectHandler::remove( ch );
    } 
    virtual void remove( Object * obj) 
    { 
        DefaultAffectHandler::remove( obj );
    } 
    virtual void remove( Room * room ) 
    { 
        DefaultAffectHandler::remove( room );
    } 
    virtual void update( Character *ch, Affect * af ) 
    { 
        DefaultAffectHandler::update( ch, af );
    } 
    virtual void update( Object *obj, Affect * af ) 
    { 
        DefaultAffectHandler::update( obj, af );
    } 
    virtual void update( Room *room, Affect * af ) 
    { 
        DefaultAffectHandler::update( room, af );
    } 
    virtual void entry( Character *ch, Affect * af )
    {
        DefaultAffectHandler::entry( ch, af );
    }
    virtual void entry( Room *room, Character *ch, Affect * af )
    {
        DefaultAffectHandler::entry( room, ch, af );
    }
    virtual void leave( Room *room, Character *ch, Affect * af )
    {
        DefaultAffectHandler::leave( room, ch, af );
    }
    virtual void look( Character *ch, Character *victim, Affect * af )
    {
        DefaultAffectHandler::look( ch, victim, af );
    }
    virtual bool smell( Character *ch, Character *victim, Affect * af )
    {
        return DefaultAffectHandler::smell( ch, victim, af );
    }
    virtual void saves( Character *ch, Character *victim, int &save, int dam_type, Affect *af )
    {
        DefaultAffectHandler::saves( ch, victim, save, dam_type, af );
    }
    virtual void stopfol( Character *ch, Affect *af )
    {
        DefaultAffectHandler::stopfol( ch, af );
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
};

#define AFFECT_DUMMY(x)         dummyAffect_ ##x## _TypeName
#define AFFECT(x) AffectHandlerTemplate<AFFECT_DUMMY(x)>

#define AFFECT_DECL(x) \
const char *AFFECT_DUMMY(x) = "AFFECT(" #x ")"; \
PluginInitializer<AFFECT(x)> dummyAffect_ ##x## _init;

#define TYPE_AFFECT(type, x) template <> type AFFECT(x)
#define VOID_AFFECT(x) TYPE_AFFECT(void, x)

// MOC_SKIP_END
#endif

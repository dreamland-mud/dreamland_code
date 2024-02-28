/* $Id: spelltemplate.h,v 1.1.2.3.10.8 2010-09-01 21:20:47 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef        __SPELLTEMPLATE_H__
#define __SPELLTEMPLATE_H__

// MOC_SKIP_BEGIN
#include "defaultspell.h"
#include "class.h"
#include "plugininitializer.h"
#include "classselfregistratorplugin.h"

template <const char *&tn, typename Target> struct SpellTemplate;
template <const char *&tn> struct SpellType;

template <const char *&tn>
struct SpellTemplate<tn, DefaultSpell> : public DefaultSpell, public ClassSelfRegistratorPlugin<tn> {
    typedef ::Pointer<SpellTemplate> Pointer;
    
    virtual void run( Character *, Character *, int, int ) { }
    virtual void run( Character *, Object *, int, int ) { }
    virtual void run( Character *, const DLString &, int, int ) { }
    virtual void run( Character *, Room *, int, int ) { }
    virtual bool apply( Character *ch, Character *victim, int level ) { return false; }    

    virtual bool spellbane( Character *ch, Character *victim ) const { 
        return DefaultSpell::spellbane( ch, victim );
    }
    virtual void utter( Character * ch ) { 
        DefaultSpell::utter( ch );
    }
    virtual bool checkPosition( Character *ch ) const { 
        return DefaultSpell::checkPosition( ch );
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
};

#define SPELL_DUMMY(x)         dummySpell_ ##x## _TypeName

#define SPELL_TYPE(x, T) \
template <> \
struct SpellType<SPELL_DUMMY(x)> { \
    typedef T Target; \
};

#define SPELL(x) SpellTemplate<SPELL_DUMMY(x), SpellType<SPELL_DUMMY(x)>::Target>

#define SPELL_DECL_T(x, T) \
const char *SPELL_DUMMY(x) = "SPELL(" #x ")"; \
SPELL_TYPE(x, T) \
PluginInitializer<SPELL(x)> dummySpell_ ##x## _init;

#define SPELL_DECL(x) SPELL_DECL_T(x, DefaultSpell)

#define TYPE_SPELL(type, x) template <> type SPELL(x)
#define VOID_SPELL(x) TYPE_SPELL(void, x)
#define BOOL_SPELL(x) TYPE_SPELL(bool, x)

// MOC_SKIP_END
#endif

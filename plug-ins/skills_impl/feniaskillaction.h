#ifndef __FENIASKILLACTIONHELPER_H__
#define __FENIASKILLACTIONHELPER_H__

#include "xmlvariablecontainer.h"
#include "xmlregister.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "native.h"
#include "reglist.h"

class Spell;
class Character;
class SpellTarget;
class DefaultSpell;
class AffectHandler;
class SkillCommand;
class DefaultSkillCommand;
class CommandTarget;
class WrapperTarget;

class FeniaSpellContext : public Scripting::NativeImpl<FeniaSpellContext>,
                          public Scripting::NativeHandler,
                          public XMLVariableContainer
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<FeniaSpellContext> Pointer;

    FeniaSpellContext();
    virtual ~FeniaSpellContext();    

    virtual void setSelf(Scripting::Object *s);
    virtual Scripting::Object *getSelf() const { return self; }

    Scripting::Object *self;
    
    /** Tier-based damage calculation. */
    void calcDamage();

    XML_VARIABLE XMLRegister thiz;
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLRegister spell;
    XML_VARIABLE XMLRegister ch;
    XML_VARIABLE XMLRegister obj;
    XML_VARIABLE XMLRegister vict;
    XML_VARIABLE XMLRegister room;
    XML_VARIABLE XMLRegister state;
    XML_VARIABLE XMLString arg;
    XML_VARIABLE XMLInteger level;
    XML_VARIABLE XMLInteger dam;
    XML_VARIABLE XMLInteger tier;
};                       

class FeniaCommandContext : public Scripting::NativeImpl<FeniaCommandContext>,
                          public Scripting::NativeHandler,
                          public XMLVariableContainer
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<FeniaCommandContext> Pointer;

    FeniaCommandContext();
    virtual ~FeniaCommandContext();    

    virtual void setSelf(Scripting::Object *s);
    virtual Scripting::Object *getSelf() const { return self; }

    Scripting::Object *self;
    
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLRegister command;
    XML_VARIABLE XMLRegister ch;
    XML_VARIABLE XMLRegister obj;
    XML_VARIABLE XMLRegister vict;
    XML_VARIABLE XMLRegister state;
    XML_VARIABLE XMLInteger level;
    XML_VARIABLE XMLInteger dam;    
    XML_VARIABLE XMLString argAll, argOne, argTwo;
};                       

class FeniaSkillActionHelper {
public:
    static void linkWrapper(Spell *);
    static void extractWrapper(Spell *);
    static void linkWrapper(AffectHandler *);
    static void extractWrapper(AffectHandler *);
    static void linkWrapper(SkillCommand *);
    static void extractWrapper(SkillCommand *);

    static bool executeSpellRun(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level);
    static bool executeSpellApply(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level, bool &rc);    
    static bool executeCommandRun(DefaultSkillCommand *cmd, Character *ch, const CommandTarget &target);
    static bool executeCommandApply(DefaultSkillCommand *cmd, Character *ch, Character *victim, int level, bool &rc);
    static bool spellHasTrigger(Spell *spell, const DLString &trigName);

private:
    static Scripting::Register createContext(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level);
    static Scripting::Register createContext(DefaultSkillCommand *cmd, Character *ch, const CommandTarget &target);    
    static Scripting::Register createContext(DefaultSkillCommand *cmd, Character *ch, Character *victim, int level);
    static DLString getMethodSuffix(::Pointer<SpellTarget> &spellTarget);
    static bool executeMethod(WrapperTarget *wtarget, const DLString &methodName, Scripting::Register ctx, bool &rc);
};



#endif 

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

    Scripting::Object *self;
    
    /** Tier-based damage calculation. */
    void calcDamage();

    /** Break circular references once spell is cast. */
    void cleanup();

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

    Scripting::Object *self;
    
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLRegister command;
    XML_VARIABLE XMLRegister ch;
    XML_VARIABLE XMLRegister obj;
    XML_VARIABLE XMLRegister vict;
    XML_VARIABLE XMLRegister state;
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

    static bool executeSpell(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level);
    static bool executeCommand(DefaultSkillCommand *cmd, Character *ch, const CommandTarget &target);
    static bool spellHasTrigger(Spell *spell, const DLString &trigName);

private:
    static FeniaSpellContext::Pointer createContext(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level);
    static FeniaCommandContext::Pointer createContext(DefaultSkillCommand *cmd, Character *ch, const CommandTarget &target);    
    static DLString getMethodName(::Pointer<SpellTarget> &spellTarget);
};



#endif 

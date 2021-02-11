#ifndef __FENIASPELLHELPER_H__
#define __FENIASPELLHELPER_H__

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

class FeniaSpellHelper {
public:
    static void linkWrapper(Spell *);
    static void extractWrapper(Spell *);
    static void linkWrapper(AffectHandler *);
    static void extractWrapper(AffectHandler *);

    static bool executeSpell(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level);
    static bool spellHasTrigger(Spell *spell, const DLString &trigName);

private:
    static FeniaSpellContext::Pointer createContext(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level);
    static DLString getMethodName(::Pointer<SpellTarget> &spellTarget);
};


#endif 

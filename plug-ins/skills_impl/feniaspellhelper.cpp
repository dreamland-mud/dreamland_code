#include <jsoncpp/json/json.h>

#include "logstream.h"
#include "feniaspellhelper.h"
#include "defaultspell.h"
#include "defaultaffecthandler.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "idcontainer.h"

#include "stringlist.h"
#include "configurable.h"
#include "skillmanager.h"
#include "skill.h"
#include "spell.h"
#include "spelltarget.h"
#include "character.h"
#include "core/object.h"
#include "room.h"
#include "act.h"
#include "dl_math.h"
#include "math_utils.h"

using namespace Scripting;
using namespace std;

// Level 1 and 110 dice pairs defined in spell/damage_tiers.json.
struct spell_damage_t
{
    int d1;
    int d110;

    void fromJson(const Json::Value &value)
    {
        d1 = d110 = 0;
        if (value.isArray() && value.size() == 2) {
            d1 = value[0].asInt();
            d110 = value[1].asInt();
            return;
        }
    }

    int valueAtLevel(int level) const
    {
        int minLevel = 1, maxLevel = MAX_LEVEL;
        int point = URANGE(minLevel, level, maxLevel);

        return linear_interpolation(
                point,
                minLevel, maxLevel, 
                d1, d110);
    }
};

json_vector<spell_damage_t> damage_tiers;
CONFIGURABLE_LOADED(spell, damage_tiers)
{
    damage_tiers.fromJson(value);
}

// Output a comma-separated list of damage dices for the given tier.
DLString print_damage_tiers(int tier, int level_step)
{
    spell_damage_t &damage = damage_tiers[tier - 1];    
    StringList dices;

    for (int lev = 0; lev <= MAX_LEVEL; lev += level_step) {
        dices.push_back(
            fmt(0, "%2d", damage.valueAtLevel(lev))
        );
    }

    return dices.join(", ");
}

void FeniaSpellHelper::linkWrapper(Spell *spell) 
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when linking spell wrapper for " << spell->getSkill()->getName() << endl;
        return;
    }

    FeniaManager::wrapperManager->linkWrapper(spell);
    if (spell->wrapper)
        LogStream::sendNotice() << "Fenia spell: linked wrapper for " << spell->getSkill()->getName() << endl;
}

void FeniaSpellHelper::extractWrapper(Spell *spell) 
{
    if (!spell->wrapper)
        return;
        
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when extracting spell wrapper for " << spell->getSkill()->getName() << endl;
        return;
    }

    spell->extractWrapper(false);
}

void FeniaSpellHelper::linkWrapper(AffectHandler *ah) 
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when linking affect handler wrapper for " << ah->getSkill()->getName() << endl;
        return;
    }

    FeniaManager::wrapperManager->linkWrapper(ah);
    if (ah->wrapper)
        LogStream::sendNotice() << "Fenia affect handler: linked wrapper for " << ah->getSkill()->getName() << endl;
}

void FeniaSpellHelper::extractWrapper(AffectHandler *ah) 
{
    if (!ah->wrapper)
        return;
        
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when extracting affect handler wrapper for " << ah->getSkill()->getName() << endl;
        return;
    }

    ah->extractWrapper(false);
}

bool FeniaSpellHelper::executeSpell(DefaultSpell *spell, Character *ch, SpellTarget::Pointer &spellTarget, int level) 
{
    // Check that a function matching this spell target (i.e. one of runVict, runArg etc)
    // is actually defined on the spell's wrapper.
    WrapperBase *wrapper = spell->getWrapper();
    if (!wrapper)
        return false;

    DLString methodName = getMethodName(spellTarget);
    if (methodName.empty())
        return false;

    IdRef methodId(methodName);
    Register method;    
    if (!wrapper->triggerFunction(methodId, method))
        return false;

    // Create run context for this spell and launch the runXXX function.
    FeniaSpellContext::Pointer ctx;
    try {
        ctx = createContext(spell, ch, spellTarget, level);
        method.toFunction()->invoke(ctx->thiz, RegisterList());
        
    } catch (const CustomException &ce) {
        // Do nothing on victim's death.

    } catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
        wrapper->croak(methodId, e);
    }

    // Clean any references that may prevent garbage collector from destroying this context object.
    if (ctx)
        ctx->cleanup();

    return true;
}

FeniaSpellContext::Pointer FeniaSpellHelper::createContext(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level) 
{
    FeniaSpellContext::Pointer ctx(NEW);
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(ctx);

    ctx->thiz = Register(ctx->self);
    ctx->name = spell->getSkill()->getName();
    ctx->spell = Register(spell->wrapper);
    ctx->ch = FeniaManager::wrapperManager->getWrapper(ch);
    ctx->level = level;
    ctx->tier = spell->tier;
    ctx->state = Register::handler<IdContainer>();
    
    switch (spellTarget->type) {
    case SpellTarget::NONE:
        ctx->arg = spellTarget->arg;
        break;
    case SpellTarget::CHAR:
        ctx->vict = FeniaManager::wrapperManager->getWrapper(spellTarget->victim);
        break;
    case SpellTarget::OBJECT:
        ctx->obj = FeniaManager::wrapperManager->getWrapper(spellTarget->obj);
        break;
    case SpellTarget::ROOM:
        ctx->room = FeniaManager::wrapperManager->getWrapper(spellTarget->room);
        break;        
    default:
        break;
    }

    ctx->calcDamage();

    return ctx;    
}

bool FeniaSpellHelper::spellHasTrigger(Spell *spell, const DLString &trigName) 
{
    WrapperBase *wrapper = spell->getWrapper();
    if (wrapper) {
        IdRef methodId(trigName);
        Register method;
        return wrapper->triggerFunction(methodId, method);
    }

    return false;
}



DLString FeniaSpellHelper::getMethodName(SpellTarget::Pointer &spellTarget) 
{
    switch (spellTarget->type) {
    case SpellTarget::NONE:   return "runArg";
    case SpellTarget::CHAR:   return "runVict";
    case SpellTarget::OBJECT: return "runObj";
    case SpellTarget::ROOM:   return "runRoom";
    default:                  return DLString::emptyString;
    }    
}

/*--------------------------------------------------------------------
 * FeniaSpellContext
 *-------------------------------------------------------------------*/

FeniaSpellContext::FeniaSpellContext() 
{
}

FeniaSpellContext::~FeniaSpellContext() 
{
}

void FeniaSpellContext::setSelf(Scripting::Object *s) 
{
    self = s;
}

void FeniaSpellContext::cleanup()
{
    thiz = Register();
    spell = Register();
    state = Register();
}

NMI_INIT(FeniaSpellContext, "контекст для вызова заклинания")

NMI_GET(FeniaSpellContext, spell, "прототип заклинания (.Spell())")
{
    return spell;
}

NMI_GET(FeniaSpellContext, ch, "персонаж, произносящий заклинание")
{
    return ch;
}

NMI_GET(FeniaSpellContext, arg, "строка, цель заклинания для runArg")
{
    return Register(level);
}

NMI_GET(FeniaSpellContext, obj, "предмет, цель заклинания для runObj")
{
    return obj;
}

NMI_GET(FeniaSpellContext, vict, "персонаж, цель заклинания для runVict - как синоним victim")
{
    return vict;
}

NMI_GET(FeniaSpellContext, victim, "персонаж, цель заклинания для runVict - как синоним vict")
{
    return vict;
}

NMI_GET(FeniaSpellContext, room, "комната, цель заклинания для runRoom")
{
    return room;
}

NMI_GET(FeniaSpellContext, level, "уровень заклинания")
{
    return Register(level);
}

NMI_SET(FeniaSpellContext, level, "уровень заклинания")
{
    level = arg.toNumber();
}

NMI_GET(FeniaSpellContext, dam, "расчетные повреждения")
{
    return Register(dam);
}

NMI_SET(FeniaSpellContext, dam, "расчетные повреждения")
{
    dam = arg.toNumber();
}

NMI_SET(FeniaSpellContext, thiz, "расчетные повреждения")
{
	thiz = arg;
}

NMI_GET(FeniaSpellContext, state, "структура для хранения временных переменных")
{
    return state;
}


void FeniaSpellContext::calcDamage()
{
    spell_damage_t &damage = damage_tiers[tier - 1];
    int d = damage.valueAtLevel(level);
    dam = dice(level, d);
}






#include <jsoncpp/json/json.h>

#include "logstream.h"
#include "feniaskillaction.h"
#include "defaultspell.h"
#include "defaultaffecthandler.h"
#include "defaultskillcommand.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "idcontainer.h"
#include "schedulerwrapper.h"

#include "stringlist.h"
#include "configurable.h"
#include "skillmanager.h"
#include "character.h"
#include "core/object.h"
#include "room.h"
#include "fight_exception.h"
#include "fight.h"
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

void FeniaSkillActionHelper::linkWrapper(Spell *spell) 
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when linking spell wrapper for " << spell->getSkill()->getName() << endl;
        return;
    }

    FeniaManager::wrapperManager->linkWrapper(spell);
    if (spell->wrapper)
        LogStream::sendNotice() << "Fenia spell: linked wrapper for " << spell->getSkill()->getName() << endl;
}

void FeniaSkillActionHelper::extractWrapper(Spell *spell) 
{
    if (!spell->wrapper)
        return;
        
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when extracting spell wrapper for " << spell->getSkill()->getName() << endl;
        return;
    }

    spell->extractWrapper(false);
}

void FeniaSkillActionHelper::linkWrapper(AffectHandler *ah) 
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when linking affect handler wrapper for " << ah->getSkill()->getName() << endl;
        return;
    }

    FeniaManager::wrapperManager->linkWrapper(ah);
    if (ah->wrapper)
        LogStream::sendNotice() << "Fenia affect handler: linked wrapper for " << ah->getSkill()->getName() << endl;
}

void FeniaSkillActionHelper::extractWrapper(AffectHandler *ah) 
{
    if (!ah->wrapper)
        return;
        
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when extracting affect handler wrapper for " << ah->getSkill()->getName() << endl;
        return;
    }

    ah->extractWrapper(false);
}

void FeniaSkillActionHelper::linkWrapper(SkillCommand *cmd) 
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when linking skill command wrapper for " << cmd->getSkill()->getName() << endl;
        return;
    }

    FeniaManager::wrapperManager->linkWrapper(cmd);
    if (cmd->wrapper)
        LogStream::sendNotice() << "Fenia skill command: linked wrapper for " << cmd->getSkill()->getName() << endl;
}

void FeniaSkillActionHelper::extractWrapper(SkillCommand *cmd) 
{
    if (!cmd->wrapper)
        return;
        
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError() << "No Fenia manager when extracting skill command wrapper for " << cmd->getSkill()->getName() << endl;
        return;
    }

    cmd->extractWrapper(false);
}

bool FeniaSkillActionHelper::executeSpellRun(DefaultSpell *spell, Character *ch, SpellTarget::Pointer &spellTarget, int level) 
{
    bool rcIgnored;
    FeniaSpellContext::Pointer ctx = createContext(spell, ch, spellTarget, level);
    // Figure out applicable runXXX method and call it, if defined on the spell wrapper.
    return executeMethod(spell, "run" + getMethodSuffix(spellTarget), ctx, rcIgnored);
}

bool FeniaSkillActionHelper::executeSpellApply(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level)
{
    bool rcIgnored;
    FeniaSpellContext::Pointer ctx = createContext(spell, ch, spellTarget, level);
    // Figure out applicable applyXXX method and call it, if defined on the spell wrapper.
    return executeMethod(spell, "apply" + getMethodSuffix(spellTarget), ctx, rcIgnored);
}

bool FeniaSkillActionHelper::executeCommandRun(DefaultSkillCommand *cmd, Character *ch, const CommandTarget &target)
{
    bool rcIgnored;
    return executeMethod(cmd, "run", createContext(cmd, ch, target), rcIgnored);
}

bool FeniaSkillActionHelper::executeCommandApply(DefaultSkillCommand *cmd, Character *ch, Character *victim, int level, bool &rc)
{
    return executeMethod(cmd, "apply", createContext(cmd, ch, victim, level), rc);
}

bool FeniaSkillActionHelper::executeMethod(WrapperTarget *wtarget, const DLString &methodName, const Scripting::Handler::Pointer &ctx, bool &rc)
{
    // Find method defined on the wrapper.
    WrapperBase *wrapper = wtarget->getWrapper();
    if (!wrapper)
        return false;

    IdRef methodId(methodName);
    Register method;
    if (!wrapper->triggerFunction(methodId, method))
        return false;

    // Invoke the function with the provided context, save its return value for further use.
    try {
        Register returnValue = method.toFunction()->invoke(Register(ctx->getSelf()), RegisterList());
        if (returnValue.type != Register::NONE)
            rc = returnValue.toBoolean();

    } catch (const CustomException &ce) {
        // Propagate exception further on victim's death.
        if (ce.message == "victim is dead")
            throw VictimDeathException();        

    } catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
        FeniaManager::getThis()->croak(0, methodId, e);
    }

    return true;
}

FeniaSpellContext::Pointer FeniaSkillActionHelper::createContext(DefaultSpell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level) 
{
    FeniaSpellContext::Pointer ctx(NEW);
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(ctx);

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

FeniaCommandContext::Pointer FeniaSkillActionHelper::createContext(DefaultSkillCommand *cmd, Character *ch, const CommandTarget &target) 
{
    FeniaCommandContext::Pointer ctx(NEW);
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(ctx);

    ctx->name = cmd->getSkill()->getName();
    ctx->command = Register(cmd->wrapper);
    ctx->ch = FeniaManager::wrapperManager->getWrapper(ch);
    ctx->state = Register::handler<IdContainer>();
    ctx->argAll = target.argAll;
    ctx->argOne = target.argOne;
    ctx->argTwo = target.argTwo;
    ctx->dam = 0;

    if (target.obj)
        ctx->obj = FeniaManager::wrapperManager->getWrapper(target.obj);
    
    if (target.vict)
        ctx->vict = FeniaManager::wrapperManager->getWrapper(target.vict);

    return ctx;        
}

FeniaCommandContext::Pointer FeniaSkillActionHelper::createContext(DefaultSkillCommand *cmd, Character *ch, Character *victim, int level)
{
    FeniaCommandContext::Pointer ctx(NEW);
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(ctx);

    ctx->name = cmd->getSkill()->getName();
    ctx->command = Register(cmd->wrapper);
    ctx->ch = FeniaManager::wrapperManager->getWrapper(ch);
    ctx->level = level;

    if (victim)
        ctx->vict = FeniaManager::wrapperManager->getWrapper(victim);

    return ctx;        
}

bool FeniaSkillActionHelper::spellHasTrigger(Spell *spell, const DLString &trigName) 
{
    WrapperBase *wrapper = spell->getWrapper();
    if (wrapper) {
        IdRef methodId(trigName);
        Register method;
        return wrapper->triggerFunction(methodId, method);
    }

    return false;
}



DLString FeniaSkillActionHelper::getMethodSuffix(SpellTarget::Pointer &spellTarget) 
{
    switch (spellTarget->type) {
    case SpellTarget::NONE:   return "Arg";
    case SpellTarget::CHAR:   return "Vict";
    case SpellTarget::OBJECT: return "Obj";
    case SpellTarget::ROOM:   return "Room";
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
    return arg;
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


NMI_INVOKE(FeniaSpellContext, start, "(func[, args...]): запустить в новом потоке функцию с аргументами")
{
    // First argument is a function to run asyncronously, all remaining optional args are function parameters.
    RegisterList::const_iterator ai = args.begin();
    if (ai == args.end())
        throw Scripting::NotEnoughArgumentsException();

    Register asyncFun = (*ai++);
    if (asyncFun.type != Register::FUNCTION)
        throw Scripting::Exception("First argument is not a function");
    
    RegisterList asyncArgs;
    asyncArgs.assign(ai, args.end( ));

    // Create and kick off new process. 
    FeniaProcess::Pointer process(NEW);
    process->args.assign(asyncArgs.begin(), asyncArgs.end());
    // Ensure that same context variables (ch, vict, skill) will be accessible from within the process function.
    process->thiz = Register(self);
    process->fun = asyncFun;
    process->name.setValue("Thread for spell '" + name + "'");

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(process);
    
    process->start();
    return Register();
}

/*--------------------------------------------------------------------
 * FeniaCommandContext
 *-------------------------------------------------------------------*/

FeniaCommandContext::FeniaCommandContext() 
{
    
}

FeniaCommandContext::~FeniaCommandContext() 
{
    
}

void FeniaCommandContext::setSelf(Scripting::Object *s) 
{
    self = s;    
}

NMI_INIT(FeniaCommandContext, "контекст для вызова команды умения")

NMI_GET(FeniaCommandContext, command, "прототип команды умения (.SkillCommand())")
{
    return command;
}

NMI_GET(FeniaCommandContext, ch, "персонаж, выполняющий команду")
{
    return ch;
}

NMI_GET(FeniaCommandContext, argAll, "аргумент команды целиком")
{
    return argAll;
}

NMI_GET(FeniaCommandContext, argOne, "первый аргумент команды")
{
    return argOne;
}

NMI_GET(FeniaCommandContext, argTwo, "второй аргумент команды")
{
    return argTwo;
}

NMI_GET(FeniaCommandContext, obj, "предмет, цель команды")
{
    return obj;
}

NMI_GET(FeniaCommandContext, vict, "персонаж, цель команды - как синоним victim")
{
    return vict;
}

NMI_GET(FeniaCommandContext, victim, "персонаж, цель команды - как синоним vict")
{
    return vict;
}

NMI_GET(FeniaCommandContext, state, "структура для хранения временных переменных")
{
    return state;
}

NMI_GET(FeniaCommandContext, level, "уровень, с которым вызвали apply")
{
    return Register(level);
}

NMI_GET(FeniaCommandContext, dam, "расчетные повреждения")
{
    return Register(dam);
}

NMI_SET(FeniaCommandContext, dam, "расчетные повреждения")
{
    dam = arg.toNumber();
}


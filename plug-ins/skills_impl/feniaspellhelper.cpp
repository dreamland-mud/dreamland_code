#include "logstream.h"
#include "feniaspellhelper.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"

#include "skillmanager.h"
#include "skill.h"
#include "spell.h"
#include "spelltarget.h"
#include "character.h"
#include "core/object.h"
#include "room.h"

using namespace Scripting;

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

bool FeniaSpellHelper::executeSpell(Spell *spell, Character *ch, SpellTarget::Pointer &spellTarget, int level) 
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
    try {
        method.toFunction()->invoke(
            createContext(spell, ch, spellTarget, level)->thiz,
            RegisterList());
        
    } catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
        wrapper->croak(methodId, e);
    }

    return true;
}

FeniaSpellContext::Pointer FeniaSpellHelper::createContext(Spell *spell, Character *ch, ::Pointer<SpellTarget> &spellTarget, int level) 
{
    FeniaSpellContext::Pointer ctx(NEW);
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(ctx);

    ctx->thiz = Register(ctx->self);
    ctx->name = spell->getSkill()->getName();
    ctx->spell = Register(spell->wrapper);
    ctx->ch = FeniaManager::wrapperManager->getWrapper(ch);
    ctx->level = level;
    
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

NMI_GET(FeniaSpellContext, dam, "расчетные повреждения")
{
    return Register(dam);
}

NMI_SET(FeniaSpellContext, dam, "расчетные повреждения")
{
    dam = arg.toNumber();
}







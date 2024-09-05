#include "behaviorwrapper.h"
#include "behavior.h"
#include "json_utils.h"

#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "idcontainer.h"
#include "lex.h"

using namespace Scripting;
using Scripting::NativeTraits;

NMI_INIT(BehaviorWrapper, "поведение")

BehaviorWrapper::BehaviorWrapper() : target(NULL)
{
}

void BehaviorWrapper::extract(bool count)
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Behavior wrapper: extract without target" << endl;
    }

    GutsContainer::extract(count);
}

void BehaviorWrapper::setSelf(Scripting::Object *s)
{
    WrapperBase::setSelf(s);

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void BehaviorWrapper::setTarget(Behavior * bhv)
{
    target = bhv;
    id = bhv->getID();
}

void BehaviorWrapper::checkTarget() const 
{
    if (zombie.getValue())
        throw Scripting::Exception("Behavior is dead");

    if (!target)
        throw Scripting::Exception("Behavior is offline");
}

Behavior * BehaviorWrapper::getTarget() const
{
    checkTarget();
    return target;
}

Register BehaviorWrapper::wrap(const DLString &name)
{
    Behavior *bhv = behaviorManager->findExisting(name);

    if (!bhv)
        throw Scripting::Exception(name + ": behavior not found");
        
    return WrapperManager::getThis()->getWrapper(bhv);
}


NMI_GET(BehaviorWrapper, name, "английское название поведения") 
{ 
    checkTarget(); 
    return Register(target->getName());
}

NMI_GET(BehaviorWrapper, nameRus, "русское название поведения с падежами") 
{ 
    checkTarget(); 
    return Register(target->nameRus);
}

NMI_GET(BehaviorWrapper, cmd, "имена команд, привязанных к поведению") 
{ 
    checkTarget(); 
    return Register(target->cmd);
}

NMI_GET(BehaviorWrapper, target, "чье поведение: obj, mob, room") 
{ 
    checkTarget(); 
    return Register(target->target);
}

NMI_GET(BehaviorWrapper, props, "Map (структура) из свойств поведения") 
{
    Register propsReg = Register::handler<IdContainer>();
    IdContainer *propsMap = propsReg.toHandler().getDynamicPointer<IdContainer>();

    for (auto p = target->props.begin(); p != target->props.end(); p++) {
        propsMap->setField(
            IdRef(p.key().asString()), 
            JsonUtils::toRegister(*p));
    }

    return propsReg;    
}

NMI_INVOKE(BehaviorWrapper, api, "(): печатает этот API")
{
    ostringstream buf;
    Scripting::traitsAPI<BehaviorWrapper>(buf);
    return Register(buf.str());
}

NMI_INVOKE(BehaviorWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime")
{
    ostringstream buf;
    traitsAPI(buf);
    return Register(buf.str());
}

NMI_INVOKE(BehaviorWrapper, clear, "(): очистка всех runtime полей")
{
    guts.clear();
    self->changed();
    return Register();
}

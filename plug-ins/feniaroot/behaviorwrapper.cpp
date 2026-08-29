#include "behaviorwrapper.h"
#include "behavior.h"
#include "setbehavior.h"
#include "affectwrapper.h"
#include "affect.h"
#include "json_utils_ext.h"

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

NMI_GET(BehaviorWrapper, description, "описание поведения")
{
    checkTarget();
    return Register(target->getDescription());
}

NMI_INVOKE(BehaviorWrapper, getDescription, "(lang): описание на языке lang (0=en,1=ru,2=ua)")
{
    checkTarget();
    return Register(target->getDescription( argnum2lang(args, 1) ));
}

NMI_INVOKE(BehaviorWrapper, getNameFor, "(lang): название на языке lang (0=en,1=ru,2=ua), fallback RU")
{
    checkTarget();
    return Register(target->getNameFor( argnum2lang(args, 1) ));
}

NMI_GET(BehaviorWrapper, cmd, "имена команд, привязанных к поведению") 
{ 
    checkTarget(); 
    return Register(target->cmd);
}

NMI_GET(BehaviorWrapper, target, "чье поведение: obj, mob, room") 
{ 
    checkTarget(); 
    return Register(target->target.name());
}

NMI_GET(BehaviorWrapper, props, "Map (структура) из свойств поведения")
{
    checkTarget();
    return JsonUtils::toIdContainer(target->props);
}

NMI_GET(BehaviorWrapper, setAffects, "список (List) аффектов, которые дает собранный набор (структура .Affect); пусто для не-наборов")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    SetBehavior *sb = dynamic_cast<SetBehavior *>(target);
    if (sb) {
        for (auto &sa: sb->affects) {
            Affect af;
            sa.fill(af);
            rc->push_back( AffectWrapper::wrap( af ) );
        }
    }

    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);
    return Register( sobj );
}

NMI_INVOKE(BehaviorWrapper, getSetMessage, "(lang): сообщение при сборке набора на языке lang (0=en,1=ru,2=ua); пусто для не-наборов")
{
    checkTarget();
    SetBehavior *sb = dynamic_cast<SetBehavior *>(target);
    if (!sb)
        return Register(DLString::emptyString);

    return Register( sb->getMsgComplete( argnum2lang(args, 1) ) );
}

NMI_GET(BehaviorWrapper, setSkills, "список (List) названий умений, которые дает собранный набор; пусто для не-наборов")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    SetBehavior *sb = dynamic_cast<SetBehavior *>(target);
    if (sb) {
        for (auto &ref: sb->grantSkills) {
            Skill *sk = ref.getElement();
            if (sk != 0)
                rc->push_back( Register( sk->getName() ) );
        }
    }

    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);
    return Register( sobj );
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

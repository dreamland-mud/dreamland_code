#include "wordeffectwrapper.h"
#include "wordeffect.h"
#include "language.h"
#include "languagemanager.h"

#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

using namespace Scripting;
using Scripting::NativeTraits;

NMI_INIT(WordEffectWrapper, "слово-эффект древнего языка (word-effect)")

WordEffectWrapper::WordEffectWrapper() : target(NULL)
{
}

void WordEffectWrapper::extract(bool count)
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Word-effect wrapper: extract without target" << endl;
    }

    GutsContainer::extract(count);
}

void WordEffectWrapper::setSelf(Scripting::Object *s)
{
    WrapperBase::setSelf(s);

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void WordEffectWrapper::setTarget(WordEffect * effect)
{
    target = effect;
    id = effect->getID();
}

void WordEffectWrapper::checkTarget() const
{
    if (zombie.getValue())
        throw Scripting::Exception("Word-effect is dead");

    if (!target)
        throw Scripting::Exception("Word-effect is offline");
}

WordEffect * WordEffectWrapper::getTarget() const
{
    checkTarget();
    return target;
}

Register WordEffectWrapper::wrap(const DLString &language, const DLString &name)
{
    Language::Pointer lang = languageManager->findLanguage(language);
    if (!lang)
        throw Scripting::Exception(language + ": language not found");

    WordEffect::Pointer effect = lang->findEffect(name);
    if (!effect)
        throw Scripting::Exception(name + ": word-effect not found in language " + language);

    return WrapperManager::getThis()->getWrapper(effect.getPointer());
}

NMI_GET(WordEffectWrapper, name, "английское название эффекта")
{
    checkTarget();
    return Register(target->getEffectName());
}

NMI_GET(WordEffectWrapper, language, "название языка, которому принадлежит эффект")
{
    checkTarget();
    return Register(target->getLanguageName());
}

NMI_GET(WordEffectWrapper, meaning, "человекочитаемый смысл эффекта")
{
    checkTarget();
    return Register(target->getMeaning());
}

NMI_INVOKE(WordEffectWrapper, api, "(): печатает этот API")
{
    ostringstream buf;
    Scripting::traitsAPI<WordEffectWrapper>(buf);
    return Register(buf.str());
}

NMI_INVOKE(WordEffectWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime")
{
    ostringstream buf;
    traitsAPI(buf);
    return Register(buf.str());
}

NMI_INVOKE(WordEffectWrapper, clear, "(): очистка всех runtime полей")
{
    guts.clear();
    self->changed();
    return Register();
}

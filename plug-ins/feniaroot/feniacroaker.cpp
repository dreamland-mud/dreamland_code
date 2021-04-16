#include "feniacroaker.h"
#include "register-impl.h"
#include "schedulerwrapper.h"
#include "wrapperbase.h"
#include "pcharacter.h"
#include "codesource.h"
#include "descriptor.h"
#include "act.h"

using namespace Scripting;

bool has_fenia_security( PCMemoryInterface *pch );

void FeniaCroaker::initialization() 
{
    FeniaManager::feniaCroaker.setPointer(this);
}

void FeniaCroaker::destruction() 
{
    FeniaManager::feniaCroaker.clear();
}

void FeniaCroaker::croak(const FeniaProcess *process, const ::Exception &e) const
{
    DLString message = 
        fmt(0, "Исключение в потоке %s:{x\n%s\n", process->name.c_str(), e.what());

    wiznet(message);
}

void FeniaCroaker::croak(const WrapperBase *wrapper, const Register &key, const ::Exception &e) const
{
    Register prog;
    DLString message;

    // Try our best to guess the codesource where the buggy code is originating from.
    if (wrapper && wrapper->triggerFunction(key, prog)) {
        const CodeSource::Pointer &codeSource = prog.toFunction()->getFunction()->source.source;    
        message = fmt(0, "Исключение при вызове [%d] %s %s:{x\n%s\n", 
                       codeSource->getId(), codeSource->name.c_str(), key.toString().c_str(), e.what());

    } else {
        message = fmt(0, "Исключение при вызове %s:{x\n%s\n", key.toString().c_str(), e.what());
    }
    
    wiznet(message);
}

void FeniaCroaker::wiznet(const DLString &message)
{
    DLString msg = fmt(0, "{CТихий голос из хрустального шара фенера: {W%s{x", message.c_str());

    for (Descriptor *d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING
            && d->character
            && d->character->getPC()
            && has_fenia_security(d->character->getPC()))
        {
            d->character->send_to(msg);
        }
    }
}


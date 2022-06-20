#include "feniacroaker.h"
#include "register-impl.h"
#include "schedulerwrapper.h"
#include "wrapperbase.h"
#include "pcharacter.h"
#include "codesource.h"
#include "websocketrpc.h"
#include "descriptor.h"
#include "act.h"

using namespace Scripting;

DLString NONCE_PLACEHOLDER = "NONCE";

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

    if (isFiltered(e))
        return;

    // Try our best to guess the codesource where the buggy code is originating from.
    if (wrapper && wrapper->triggerFunction(key, prog)) {
        const CodeSource::Pointer &codeSource = prog.toFunction()->getFunction()->source.source;    
        DLString messageFormat = 
            "Исключение при вызове [" +  
            web_cmd_placeholder("cs web $1", "%d", NONCE_PLACEHOLDER) + 
            "] %s %s:{x\n%s\n";
        message = fmt(0, messageFormat.c_str(), 
                       codeSource->getId(), codeSource->name.c_str(), key.toString().c_str(), e.what());

    } else {
        message = fmt(0, "Исключение при вызове %s:{x\n%s\n", key.toString().c_str(), e.what());
    }
    
    wiznet(message);
}

void FeniaCroaker::wiznet(const DLString &exceptionMessage)
{
    DLString crystalOrbMessage = fmt(0, "{CТихий голос из хрустального шара фенера: {W%s{x", exceptionMessage.c_str());

    for (Descriptor *d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING
            && d->character
            && d->character->getPC()
            && has_fenia_security(d->character->getPC()))
        {
            DLString messageWithNonce = crystalOrbMessage;
            messageWithNonce.replaces(NONCE_PLACEHOLDER, d->websock.nonce);
            d->character->send_to(messageWithNonce);
        }
    }
}

// Don't spam about certain exceptions.
bool FeniaCroaker::isFiltered(const ::Exception &e)
{
    if (e.getMessage() == "victim is dead")
        return true;

    return false;
}
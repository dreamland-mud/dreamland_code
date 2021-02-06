#include "pluginwrapperimpl.h"
#include "codesource.h"
#include "descriptor.h"
#include "act.h"

using namespace Scripting;

bool has_fenia_security( PCMemoryInterface *pch );

void fenia_wiznet(const WrapperBase *wrapper, const Register &key, const ::Exception &e)
{
    Register prog;

    if (!wrapper->triggerFunction(key, prog))
        return;

    const CodeSource::Pointer &codeSource = prog.toFunction()->getFunction()->source.source;
    DLString msg = fmt(0, "{CТихий голос из хрустального шара фенера: {WИсключение при вызове [%d] %s %s:{x\n%s\n", 
                       codeSource->getId(), codeSource->name.c_str(), key.toString().c_str(), e.what());

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

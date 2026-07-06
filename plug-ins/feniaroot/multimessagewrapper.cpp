/* Dreamland trilinguality -- Fenia wrapper carrying a MultiMessage.
 * Wave 3 / Trello 2594.
 */
#include "register-impl.h"
#include "multimessagewrapper.h"

const DLString MultiMessageWrapper::TYPE = "MultiMessageWrapper";

Scripting::Register MultiMessageWrapper::getField(const Scripting::Register &)
{
    return Scripting::Register();
}

Scripting::Register MultiMessageWrapper::callMethod(const Scripting::Register &, const Scripting::RegisterList &)
{
    return Scripting::Register();
}

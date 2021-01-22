#include "feniaspellhelper.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "subr.h"
#include "characterwrapper.h"
#include "objectwrapper.h"

#include "core/object.h"
#include "character.h"
#include "def.h"

using Scripting::NativeTraits;

NMI_INVOKE(FeniaSpellContext, msgChar, "(fmt[,args]): выдать сообщение кастеру")
{
    DLString msg = args2string(args);
    Character *ch = arg2character(this->ch);
    ::Object *obj = arg2item(this->obj);
    ch->pecho(msg.c_str(), ch, obj);
    return Register();
}

#ifndef FENIA_UTILS
#define FENIA_UTILS

#include "stringset.h"

class WrapperBase;
class Object;

namespace Scripting {
    class RegisterList;
    class Register;
}

/** Call a global Fenia trigger with given name, argument format and arguments. */
bool gprog(const DLString &trigName, const char *fmt, ...);

/** Call a trigger with given name and args on an instance (mob, item, room) or its prototype (mob index data etc). */
bool fenia_trigger(Scripting::Register &rc, const DLString &trigName, const Scripting::RegisterList &args, WrapperBase *instance, WrapperBase *proto);

// Collect all triggers on obj/indexdata/behaviors in the form of "use", "equip" etc.
StringSet trigger_labels(Object *obj);

// Return name of this trigger w/o 'on' or 'post' prefix
DLString trigger_type(const DLString &constTrigger);

#endif


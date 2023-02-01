#ifndef FENIA_UTILS
#define FENIA_UTILS

class DLString;
class WrapperBase;
namespace Scripting {
    class RegisterList;
}

/** Call a global Fenia trigger with given name, argument format and arguments. */
bool gprog(const DLString &trigName, const char *fmt, ...);

/** Call a trigger with given name and args on an instance (mob, item, room) or its prototype (mob index data etc). */
bool fenia_trigger(const DLString &trigName, const Scripting::RegisterList &args, WrapperBase *instance, WrapperBase *proto);

#endif


#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "logstream.h"
#include "regcontainer.h"

/**
 * Invoke Fenia function defined in .tmp.trigger_handler,
 * passing two arguments:
 * - trigger name (e.g. onDeath, onLore, onReset)
 * - a list of trigger parameters (ch, mob, obj, etc)
 */
bool gprog(const DLString &trigName, const char *fmt, ...)
{
    static Scripting::IdRef ID_TMP("tmp"), ID_TRIG_HNDL("trigger_handler");

    if (!FeniaManager::wrapperManager)
        return false;

    try {
        va_list ap;
        Scripting::RegisterList registerList;
        // Locate trigger_handler function; exception is thrown if not found.
        Scripting::Register tmp = *Scripting::Context::root[ID_TMP];
        Scripting::Register trigger_handler = *tmp[ID_TRIG_HNDL];

        if (trigger_handler.type != Register::FUNCTION)
            return false;

        // Collect *trigger* arguments into registerList, based on the argument
        // format string "CC", "CO" and so on.        
        va_start(ap, fmt);
        WrapperBase::triggerArgs(registerList, fmt, ap);
        va_end(ap);

        // Convert RegisterList into RegList, suitable to be passed as one of the parameters
        // to the trigger_handler.
        RegList::Pointer list(NEW);
        for (RegisterList::const_iterator r = registerList.begin(); r != registerList.end(); r++)
            list->push_back(*r);

        Scripting::Object *listObj = &Scripting::Object::manager->allocate();
        listObj->setHandler(list);

        // Collect two trigger_handler arguments into RegisterList.
        Scripting::RegisterList args;
        args.push_front(trigName);
        args.push_back(listObj);

        // Invoke trigger handler ignoring its result.
        trigger_handler.toFunction()->invoke(tmp, args);
        return true;
    }
    catch (const Scripting::Exception &e) {
        LogStream::sendWarning( ) << "global trigger: " << e.what( ) << endl;
        return false;
    }
}


    


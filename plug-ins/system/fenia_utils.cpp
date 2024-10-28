#include "lex.h"
#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "logstream.h"
#include "regcontainer.h"
#include "core/object.h"
#include "behavior.h"
#include "fenia_utils.h"
#include "merc.h"
#include "def.h"

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
    catch (const ::Exception &e) {
        // On error, complain to the logs and to all immortals in the game.
        FeniaManager::getThis()->croak(0, Scripting::Register(trigName), e);
        return false;
    }
}

/** 
 * Call a trigger with given name and args on an instance (mob, item, room) or its prototype (mob index data etc). 
 * Return 'true' if one of the triggers also returns true. Invokation result is saved in 'rc' register.
 */
bool fenia_trigger(Register &rc, const DLString &trigName, const Scripting::RegisterList &args, WrapperBase *instance, WrapperBase *proto)
{
    // From "Death" trigger name form "onDeath" and "postDeath" names.
    Scripting::IdRef onTrigName("on" + trigName);
    Scripting::IdRef postTrigName("post" + trigName);

    // Collect arguments for the instance trigger call: everything but trigName.
    // Collect arguments for the index data call: 'this' then everything but trigName.
    RegisterList trigArgs = args, protoTrigArgs = args;
    protoTrigArgs.push_front(Register(instance->getSelf()));

    // Keep trigger functions here.
    Register onProg, postProg;

    // Execute postXXX trigger on the instance, if defined.
    if (instance->triggerFunction(postTrigName, postProg)) {
        instance->postpone(postTrigName, postProg, trigArgs);
    }

    // Execute postXXX trigger on index data, if defined.
    if (proto && proto->triggerFunction(postTrigName, postProg)) {
        proto->postpone(postTrigName, postProg, protoTrigArgs);
    }

    // Execute onXXX trigger on the instance and return if it returns true.
    if (instance->triggerFunction(onTrigName, onProg)) {
        rc = onProg.toFunction()->invoke(Register(instance->getSelf()), trigArgs);    

        if (rc.type != Register::NONE && rc.type != Register::STRING && rc.toBoolean())
            return true;
    } 

    // Execute onXXX trigger on index data and return if it returns true.
    if (proto && proto->triggerFunction(onTrigName, onProg)) {
        rc = onProg.toFunction()->invoke(Register(proto->getSelf()), protoTrigArgs);    

        if (rc.type != Register::NONE && rc.type != Register::STRING && rc.toBoolean())
            return true;
    }

    return false;

}

DLString trigger_type(const DLString& constTrigger)
{
    static DLString ON_ID = "on";
    static DLString POST_ID = "post";
    DLString trig = constTrigger;

    if (ON_ID.strPrefix(trig))
        trig = trig.substr(ON_ID.size());
    else if (POST_ID.strPrefix(trig))
        trig = trig.substr(POST_ID.size());

    return trig;
}


StringSet trigger_labels(Object* obj)
{
    StringSet triggers, misc;
    StringSet result;

    // Collect all onXXX, postXXX triggers defined on object itself
    WrapperBase *objWrapper = get_wrapper(obj->wrapper);
    if (objWrapper)
        objWrapper->collectTriggers(triggers, misc);
    
    // Collect all onXXX, postXXX triggers defined on index data.
    WrapperBase *ndxWrapper = get_wrapper(obj->pIndexData->wrapper);
    if (ndxWrapper)
        ndxWrapper->collectTriggers(triggers, misc);

    // Collect all triggers defined on each of the behaviors.
    for (auto &b: obj->pIndexData->behaviors.toSet()) {
        WrapperBase *bhvWrapper = behaviorManager->find(b)->getWrapper();
        if (bhvWrapper)
            bhvWrapper->collectTriggers(triggers, misc);
    }

    for (auto &t: triggers) {
        result.insert(
            trigger_type(t).toLower());
    }

    // Check for common triggers from legacy behaviors
    if (obj->behavior) {
        static list<DLString> legacyTriggers = {"use", "examine", "command"};
        for (auto &legacy: legacyTriggers)
            if (obj->behavior->hasTrigger(legacy))
                result.insert(legacy);    
    }

    LogStream::sendNotice() << "labels " << result.toString() << " for " << obj->getShortDescr('1', LANG_DEFAULT) << endl;
    return result;
}


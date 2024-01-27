#ifndef FENIATRIGGERS_H
#define FENIATRIGGERS_H

#include <map>
#include <sstream>
#include "plugin.h"
#include "dlstring.h"
#include "oneallocate.h"
#include "xmlindexdata.h"
#include "wrappermanager.h"
#include "wrapperbase.h"

using namespace std;

class PCharacter;
class SkillAction;
class DefaultSpell;
class DefaultAffectHandler;
class DefaultSkillCommand;
class WrappedCommand;

bool stringIsCapitalized(const DLString &str);
DLString triggerType(const DLString &name);

class FeniaTriggerLoader : public Plugin, public OneAllocate {
public:
    typedef map<DLString, DLString> TriggerContent;
    typedef map<DLString, TriggerContent> IndexTriggers;

    FeniaTriggerLoader();
    virtual ~FeniaTriggerLoader();
    virtual void initialization();
    virtual void destruction();

    bool openEditor(PCharacter *ch, XMLIndexData &indexData, const DLString &constArguments) const;
    bool openEditor(PCharacter *ch, DefaultSpell *spell, const DLString &constArguments) const;
    bool openEditor(PCharacter *ch, DefaultAffectHandler *ah, const DLString &constArguments) const;
    bool openEditor(PCharacter *ch, DefaultSkillCommand *cmd, const DLString &constArguments) const;
    bool openEditor(PCharacter *ch, WrappedCommand *cmd, const DLString &constArguments) const;

    void showTriggers(PCharacter *ch, DefaultSpell *spell) const;
    void showTriggers(PCharacter *ch, WrapperBase *wrapper, const DLString &indexType) const;
    
    bool clearTrigger(Scripting::Object *wrapper, const DLString &trigName) const;
    bool clearTriggers(Scripting::Object *wrapper) const;

protected:
    bool editExisting(Character *ch, Scripting::Register &retval) const;
    bool findExample(Character *ch, const DLString &methodName, const DLString &indexType, DLString &tmpl) const;
    void loadFolder(const DLString &indexType);
    bool checkWebsock(Character *ch) const;

    template <typename WT>
    Scripting::Register getMethodForName(WT *target, const DLString &methodName) const
    {
        Scripting::Register retval;
        Scripting::Register w = WrapperManager::getThis()->getWrapper(target);
        if (w.type == Scripting::Register::NONE)
            return retval;
            
        WrapperBase *base = get_wrapper(w.toObject());
        if (!base)
            return retval;

        Scripting::IdRef methodId(methodName);
        retval = base->getField(methodId);
        return retval;
    }

    vector<DLString> createSkillActionParams(
        Character *ch, const DLString &actionType, SkillAction *action, const DLString &methodName) const;

    vector<DLString> createCommandParams(
        Character *ch, WrappedCommand *cmd, const DLString &methodName) const;

    IndexTriggers indexTriggers;
};

extern FeniaTriggerLoader *feniaTriggers;
#endif
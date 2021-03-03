#ifndef FENIATRIGGERS_H
#define FENIATRIGGERS_H

#include <map>
#include "plugin.h"
#include "dlstring.h"
#include "oneallocate.h"
#include "xmlindexdata.h"

using namespace std;

class PCharacter;
class DefaultSpell;
class DefaultAffectHandler;

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
    void showAvailableTriggers(PCharacter *ch, const DLString &indexType) const;
    void showAvailableTriggers(PCharacter *ch, DefaultSpell *spell) const;
    void showAvailableTriggers(PCharacter *ch, DefaultAffectHandler *ah) const;
    void showAssignedTriggers(PCharacter *ch,  Scripting::Object *wrapper) const;
    bool clearTrigger(Scripting::Object *wrapper, const DLString &trigName) const;

protected:
    bool editExisting(Character *ch, Scripting::Register &retval) const;
    bool findExample(Character *ch, const DLString &methodName, const DLString &indexType, DLString &tmpl) const;
    void loadFolder(const DLString &indexType);

    IndexTriggers indexTriggers;
};

extern FeniaTriggerLoader *feniaTriggers;
#endif
#ifndef FENIATRIGGERS_H
#define FENIATRIGGERS_H

#include <map>
#include "plugin.h"
#include "dlstring.h"
#include "oneallocate.h"
#include "xmlindexdata.h"

using namespace std;

class PCharacter;

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
    void showAvailableTriggers(PCharacter *ch, const DLString &indexType) const;

protected:
    void loadFolder(const DLString &indexType);

    IndexTriggers indexTriggers;
};

extern FeniaTriggerLoader *feniaTriggers;
#endif
#ifndef FENIATRIGGERS_H
#define FENIATRIGGERS_H

#include <map>
#include "plugin.h"
#include "dlstring.h"
#include "oneallocate.h"
#include "xmlindexdata.h"

using namespace std;

class PCharacter;

class FeniaTriggerLoader : public Plugin, public OneAllocate {
public:
    typedef map<DLString, DLString> TriggerContent;

    FeniaTriggerLoader();
    virtual ~FeniaTriggerLoader();
    virtual void initialization();
    virtual void destruction();

    bool openEditor(PCharacter *ch, XMLIndexData &indexData, const DLString &constArguments) const;
    void showAvailableTriggers(PCharacter *ch) const;

protected:
    TriggerContent triggers;
};

extern FeniaTriggerLoader *feniaTriggers;
#endif
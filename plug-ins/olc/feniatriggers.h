#ifndef FENIATRIGGERS_H
#define FENIATRIGGERS_H

#include <map>
#include "plugin.h"
#include "dlstring.h"
#include "oneallocate.h"

using namespace std;

class FeniaTriggerLoader : public Plugin, public OneAllocate {
public:
    typedef map<DLString, DLString> TriggerContent;

    FeniaTriggerLoader();
    virtual ~FeniaTriggerLoader();
    virtual void initialization();
    virtual void destruction();
    const DLString &getContent(const DLString &name) const;
    bool hasTrigger(const DLString &name) const;

protected:
    TriggerContent triggers;
};

extern FeniaTriggerLoader *feniaTriggers;
#endif
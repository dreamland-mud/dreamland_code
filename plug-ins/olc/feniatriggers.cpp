#include "feniatriggers.h"
#include "plugininitializer.h"
#include "dlfileloader.h"

FeniaTriggerLoader *feniaTriggers = 0;

FeniaTriggerLoader::FeniaTriggerLoader()
{
    checkDuplicate( feniaTriggers );
    feniaTriggers = this;
}
FeniaTriggerLoader::~FeniaTriggerLoader()
{
    feniaTriggers = 0;
}

void FeniaTriggerLoader::initialization()
{
    DLFileLoader loader("fenia.examples", "");
    loader.loadAll();

    DLFileLoader::Files::const_iterator f;
    DLFileLoader::Files files = loader.getAll();
    for (f = files.begin(); f != files.end(); f++)
        triggers[f->first] = f->second.content;
}

void FeniaTriggerLoader::destruction()
{

}

static DLString normalizeTriggerName(const DLString &name) 
{
    DLString key = name.toLower();
    static const DLString ON("on");
    static const DLString POST("post");

    if (ON.strPrefix(key))
        key = key.substr(ON.size());
    else if (POST.strPrefix(key))
        key = key.substr(POST.size());

    return key;
}

const DLString &FeniaTriggerLoader::getContent(const DLString &name) const
{
    TriggerContent::const_iterator t = triggers.find(normalizeTriggerName(name));
    if (t == triggers.end())
        return DLString::emptyString;
    return t->second;
}

bool FeniaTriggerLoader::hasTrigger(const DLString &name) const
{
    DLString key = normalizeTriggerName(name);
    if (key == name)
        return false;
    return triggers.find(key) != triggers.end();
}



PluginInitializer<FeniaTriggerLoader> initFeniaTriggerLoader;
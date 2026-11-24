#include "configfolder.h"
#include "logstream.h"
#include "dldirectory.h"
#include "dlfile.h"
#include "exceptiondbio.h"
#include "exceptiondbioeof.h"
#include "plugininitializer.h"
#include "dreamland.h"

std::list<ConfigFolder *> ConfigFolder::folders;

ConfigFolder::ConfigFolder(const DLString &folder) : folder(folder)
{
}

ConfigFolder::~ConfigFolder()
{
}

void ConfigFolder::initialization()
{
    folders.push_back(this);

    // A prior instance may have left entries behind if its destruction() never ran
    // before a reload; configReg->add (emplace) would then keep the stale copy.
    unloadAll();

    try {
        DLDirectory dir(dreamland->getTableDir(), folder);
        dir.open();

        for ( ;; ) {
            DLFile local = dir.nextTypedEntry(".json"); // "midgaard.are.json", EOF throws
            DLString name = local.getFileName();         // "midgaard.are"
            if (!name.empty())
                loadFile(folder + "/" + name);
        }
    }
    catch (const ExceptionDBIOEOF &) {
        // normal end of directory
    }
    catch (const ExceptionDBIO &ex) {
        // A folder that does not exist yet is not an error: it simply has no files.
        LogStream::sendNotice() << "config folder " << folder << ": " << ex.what() << endl;
    }
}

void ConfigFolder::destruction()
{
    unloadAll();
    folders.remove(this);
}

void ConfigFolder::unloadAll()
{
    if (!configReg)
        return;

    DLString prefix = folder + "/";
    for (auto &path: configReg->getPaths())
        if (path.compare(0, prefix.size(), prefix) == 0) {
            Configurable::Pointer cfg = configReg->get(path);
            if (cfg)
                cfg->unload();
        }
}

ConfigFolder * ConfigFolder::owner(const DLString &path)
{
    for (auto f: folders) {
        DLString prefix = f->getFolder() + "/";
        if (path.size() <= prefix.size() || path.compare(0, prefix.size(), prefix) != 0)
            continue;

        // Only files directly inside the folder: no subfolders, no "..".
        if (path.find('/', prefix.size()) != DLString::npos)
            continue;

        return f;
    }

    return NULL;
}

bool ConfigFolder::onDisk(const DLString &path) const
{
    return DLFile(dreamland->getTableDir(), path, ".json").exist();
}

Configurable::Pointer ConfigFolder::loadFile(const DLString &path)
{
    Configurable::Pointer cfg(NEW);
    cfg->setPath(path);
    cfg->load();

    if (!configReg)
        return Configurable::Pointer();

    return configReg->get(path);
}

/** Lines of ambient chatter per area and the bards' ballads (dreamland_world/ambient). */
class AmbientFolder : public ConfigFolder {
public:
    AmbientFolder() : ConfigFolder("ambient") { }
};

/** Character creation descriptions per race (dreamland_world/descriptions), read by the nanny. */
class DescriptionsFolder : public ConfigFolder {
public:
    DescriptionsFolder() : ConfigFolder("descriptions") { }
};

PluginInitializer<AmbientFolder> initAmbientFolder;
PluginInitializer<DescriptionsFolder> initDescriptionsFolder;

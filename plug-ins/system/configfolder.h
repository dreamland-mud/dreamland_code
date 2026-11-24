#ifndef CONFIGFOLDER_H
#define CONFIGFOLDER_H

#include <list>
#include <jsoncpp/json/json.h>
#include "plugin.h"
#include "configurable.h"

/**
 * A folder of JSON files under the table directory, each registered in configReg
 * as "<folder>/<file name without .json>". Names may hold dots, spaces and hyphens,
 * which CONFIGURABLE_DECL cannot paste into a C symbol, and a new file needs no
 * C++ line. The folder is scanned when the plugin loads; `fedit reload` registers
 * a file added later and unloads one deleted from disk (see owner()).
 */
class ConfigFolder : public virtual Plugin {
public:
    typedef ::Pointer<ConfigFolder> Pointer;

    ConfigFolder(const DLString &folder);
    virtual ~ConfigFolder();

    virtual void initialization();
    virtual void destruction();

    const DLString & getFolder() const { return folder; }

    /** The loaded folder that holds this config path ("ambient/midgaard.are"), or NULL. */
    static ConfigFolder * owner(const DLString &path);

    /** True when the JSON file behind this config path exists on disk. */
    bool onDisk(const DLString &path) const;

    /** Register and load one file of the folder. Returns the registered entry,
     *  or an empty pointer when the file did not parse. */
    Configurable::Pointer loadFile(const DLString &path);

private:
    void unloadAll();

    DLString folder;

    static std::list<ConfigFolder *> folders;
};

#endif

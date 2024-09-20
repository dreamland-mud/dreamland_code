#include <fstream>
#include <sstream>
#include "json_utils_ext.h"
#include "logstream.h"
#include "configurable.h"
#include "dreamland.h"
#include "dlfilestream.h"
#include "iconvmap.h"

static IconvMap utf2koi("utf-8", "koi8-u//IGNORE");
static IconvMap koi2utf("koi8-u", "utf-8");

Configurable::~Configurable()
{
}

DLString Configurable::getAbsolutePath() const
{
    DLFile file(dreamland->getTableDir(), getPath(), ".json");
    return file.getAbsolutePath();
}

void Configurable::load()
{
    try {
        ifstream ifs(getAbsolutePath());
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        
        setText(
            utf2koi(buffer.str()));
        
        loaded();

        LogStream::sendNotice() << "Configurable " << getPath() << " loaded with " << value.size() << " entries." << endl;

        configReg->add(Pointer(this));

    } catch (const std::exception &ex) {
        LogStream::sendError() << getPath() << ex.what() << endl;
    }
}

void Configurable::setText(const DLString &text)
{
    this->text = text;
    JsonUtils::fromString(text, value);
}

void Configurable::unload()
{
    unloaded();
    configReg->remove(Pointer(this));
}

void Configurable::save()
{
    try {
        ofstream ofs(getAbsolutePath());
        ofs << koi2utf(text);
        
        loaded();

        LogStream::sendNotice() << "Configurable " << getPath() << " updated with " << value.size() << " entries." << endl;

    } catch (const std::exception &ex) {
        LogStream::sendError() << getPath() << ":" << ex.what() << endl;
    }
}

void ConfigurablePlugin::initialization()
{
    load();
}

void ConfigurablePlugin::destruction()
{
    unload();
}

ConfigurableRegistry* configReg = NULL;

ConfigurableRegistry::ConfigurableRegistry( ) 
{
    checkDuplicate( configReg );
    configReg = this;
}

ConfigurableRegistry::~ConfigurableRegistry( )
{
    configReg = NULL;
}

void ConfigurableRegistry::add(Configurable::Pointer cfg)
{
    elements.emplace(cfg->getPath(), cfg);
}

void ConfigurableRegistry::remove(Configurable::Pointer cfg)
{
    auto i = elements.find(cfg->getPath());
    if (i != elements.end())
        elements.erase(i);
}

Configurable::Pointer ConfigurableRegistry::get(const DLString &path) const
{
    auto i = elements.find(path);
    if (i != elements.end())
        return i->second;

    return Configurable::Pointer();
}

std::list<Configurable::Pointer> ConfigurableRegistry::getAll(const DLString &path) const
{
    std::list<Configurable::Pointer> all;

    if (path.empty())
        return all;

    for (auto &e: elements)
        if (e.first.find(path) != DLString::npos)
            all.push_back(e.second);

    return all;
}

std::list<DLString> ConfigurableRegistry::getPaths() const
{
    std::list<DLString> paths;
    for (auto &e: elements)
        paths.push_back(e.first);

    return paths;
}

void ConfigurableRegistry::initialization()
{

}

void ConfigurableRegistry::destruction()
{

}

PluginInitializer<ConfigurableRegistry> initConfigurableRegistry;

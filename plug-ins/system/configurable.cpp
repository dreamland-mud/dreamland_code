#include <fstream>
#include <jsoncpp/json/json.h>
#include <sstream>
#include "logstream.h"
#include "configurable.h"
#include "dreamland.h"
#include "dlfilestream.h"

DLString Configurable::getAbsolutePath() const
{
    DLFile file(dreamland->getTableDir(), getPath(), ".json");
    return file.getAbsolutePath();
}

void Configurable::initialization()
{
    try {
        Json::Value value;

        ifstream ifs(getAbsolutePath());
        ifs >> value;

        loaded(value);
        LogStream::sendNotice() << "Configurable " << getPath() << " loaded with " << value.size() << " entries." << endl;

        configReg->add(Pointer(this));

    } catch (const std::exception &ex) {
        LogStream::sendError() << ex.what() << endl;
    }
}

void Configurable::refresh(const DLString &text)
{
    try {
        ofstream ofs(getAbsolutePath());
        ofs << text;
        
        Json::Value value;
        Json::Reader reader;
        reader.parse(text, value);

        loaded(value);

        LogStream::sendNotice() << "Configurable " << getPath() << " updated with " << value.size() << " entries." << endl;

    } catch (const std::exception &ex) {
        LogStream::sendError() << getPath() << ":" << ex.what() << endl;
    }
}

DLString Configurable::getText() const
{
    try {
        ifstream ifs(getAbsolutePath());
        std::stringstream buffer;
        buffer << ifs.rdbuf();
        return buffer.str();

    } catch (const std::exception &ex) {
        LogStream::sendError() << getPath() << ":" << ex.what() << endl;
        return "ERROR";
    }
}

void Configurable::destruction()
{
    unloaded();
    configReg->remove(Pointer(this));
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


bool json_validate_text(const DLString &text, ostringstream &errbuf)
{
    try {
        Json::Value value;
        Json::Reader reader;

        if (reader.parse(text, value))
            return true;

        errbuf << reader.getFormattedErrorMessages();

    } catch (const std::exception &ex) {
        errbuf << ex.what();
    }

    return false;  
}

#include <fstream>
#include <jsoncpp/json/json.h>
#include <sstream>
#include "logstream.h"
#include "configurable.h"
#include "dreamland.h"
#include "dlfilestream.h"

void Configurable::initialization()
{
    DLFile file(dreamland->getTableDir(), getPath(), ".json");
    if (!file.exist()) {
        LogStream::sendError() << "Configurable file not found: " << file.getAbsolutePath() << endl;
        return;
    }

    Json::Value value;
    try {
        ifstream ifs(file.getAbsolutePath());
        ifs >> value;
    } catch (const std::exception &ex) {
        LogStream::sendError() << "Configurable " << getPath() << " error: " << ex.what() << endl;
        return;
    }

    loaded(value);

    LogStream::sendNotice() << "Loaded " << getPath() << ".json with " << value.size() << " values" << endl;

    configReg->add(Pointer(this));
}

DLString Configurable::getText() const
{
    ostringstream text;

    try {
        DLFileStream fs(dreamland->getTableDir(), getPath(), ".json");
        fs.toStream(text);

    } catch (const std::exception &ex) {
        LogStream::sendError() << "Configurable " << getPath() << " error: " << ex.what() << endl;
        text << "ERROR: " << ex.what() << endl;
    }

    return text.str();
}

bool Configurable::validate(const DLString &text, ostringstream &errbuf) const
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

void Configurable::setText(const DLString &text)
{
    Json::Value value;

    try {
        DLFileStream fs(dreamland->getTableDir(), getPath(), ".json");
        fs.fromString(text);

        Json::Reader reader;
        reader.parse(text, value);

    } catch (const std::exception &ex) {
        LogStream::sendError() << "Configurable " << getPath() << " error: " << ex.what() << endl;
        return;
    }

    loaded(value);

    LogStream::sendNotice() << "Configurable " << getPath() << " updated with new data." << endl;
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


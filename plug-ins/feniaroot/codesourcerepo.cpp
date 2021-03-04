#include <filesystem>

#include "logstream.h"
#include "codesourcerepo.h"
#include "codesource.h"
#include "plugininitializer.h"
#include "iconvmap.h"
#include "dlfilestream.h"
#include "codesource.h"
#include "fenia/register-impl.h"
#include "dreamland.h"

using namespace Scripting;
namespace fs = std::filesystem;
static IconvMap koi2utf("koi8-r", "utf-8");
static IconvMap utf2koi("utf-8", "koi8-r");

PluginInitializer<CodeSourceRepo> initCSRepo;

CodeSourceRepo * CodeSourceRepo::thisClass = 0;

CodeSourceRepo::CodeSourceRepo() 
{
    checkDuplicate(thisClass);
    thisClass = this;
    
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(spell/[a-z ]{2,})/(run[a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(areas/[-a-z0-9]{2,}.are/[a-z]+)/([0-9.a-zA-Z ]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(areas/[-a-z0-9]{2,}.are)/([a-z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(quest)/([0-9.a-zA-Z :]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(gquest)/([0-9.a-zA-Z :]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(behavior)/([0-9.a-zA-Z ]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(global/on[A-Z][a-z]+)/([_a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(global/post[A-Z][a-z]+)/([_a-zA-Z]+)$", true));
}

CodeSourceRepo::~CodeSourceRepo() 
{
    thisClass = 0;    
}


void CodeSourceRepo::save(Scripting::CodeSource &cs) 
{
    DLString folderName;
    DLString fileName;

    for (auto &pattern: subjPatterns) {
        RegExp::MatchVector matches = pattern->subexpr(cs.name.c_str());
        if (!matches.empty()) {
            folderName = matches.at(0);
            fileName = matches.at(1);
            break;
        }
    }

    if (folderName.empty() || fileName.empty()) {
        LogStream::sendWarning() << "CS repo: unknown pattern for " << cs.name << endl;
        return;
    }

    fs::path path = dreamland->getTableDir().getAbsolutePath().c_str();
    path /= "fenia";
    path /= folderName.c_str();
    fs::create_directories(path);

    path /= fileName.c_str();
    ofstream os(path);
    os << koi2utf(cs.content);

    if (!os) {
        LogStream::sendSystem() << "CS repo: error saving codesource " << cs.name << endl;
    } else 
        LogStream::sendNotice() << "CS repo: saved " << cs.name << endl;
}

void CodeSourceRepo::saveAll() 
{
    CodeSource::Manager::iterator i;

    for(i = CodeSource::manager->begin( );i != CodeSource::manager->end( ); i++) {
        save(*i);
    }
}

bool CodeSourceRepo::read(const DLString &csName) 
{
    stringstream content;

    try {
        fs::path path = dreamland->getTableDir().getAbsolutePath().c_str();
        path /= "fenia";
        path /= csName.c_str();

        ifstream ifs(path);
        content << ifs.rdbuf();

        if (!ifs) {
            LogStream::sendSystem() << "CS repo: read failed " << csName << endl;
            return false;
        }

    } catch (const std::exception &ex) {
        LogStream::sendError() << "CS repo: read " << csName << ": " << ex.what() << endl;
        return false;
    }

    try {
        CodeSource &cs = CodeSource::manager->allocate();
        cs.author = "Chronos";
        cs.name = csName;
        cs.content = utf2koi(content.str());

        cs.eval();

    } catch(const ::Exception& e) {
        LogStream::sendError() << "CS repo: read " << csName << ": " << e.what() << endl;
        return false;
    }

    return true;
}

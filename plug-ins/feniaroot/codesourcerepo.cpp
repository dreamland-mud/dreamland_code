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

PluginInitializer<CodeSourceRepo> initCSRepo;

CodeSourceRepo * CodeSourceRepo::thisClass = 0;

CodeSourceRepo::CodeSourceRepo() 
{
    checkDuplicate(thisClass);
    thisClass = this;
    
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(spell/[a-z ]{2,})/(run[a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(areas/[a-z0-9]{2,}.are/[a-z]+)/([0-9.a-zA-Z ]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(areas/[a-z0-9]{2,}.are)/([a-z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(quest)/([0-9.a-zA-Z :]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(gquest)/([0-9.a-zA-Z :]+)$", true));
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

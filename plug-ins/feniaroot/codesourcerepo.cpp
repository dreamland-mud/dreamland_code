#include <filesystem>

#include "logstream.h"
#include "codesourcerepo.h"
#include "codesource.h"
#include "plugininitializer.h"
#include "iconvmap.h"
#include "dlfilestream.h"
#include "codesource.h"
#include "fenia/register-impl.h"
#include "dl_ctype.h"
#include "dreamland.h"

using namespace Scripting;
namespace fs = std::filesystem;

static IconvMap koi2utf("koi8-u", "utf-8");
static IconvMap utf2koi("utf-8", "koi8-u");

PluginInitializer<CodeSourceRepo> initCSRepo;

CodeSourceRepo * CodeSourceRepo::thisClass = 0;

CodeSourceRepo::CodeSourceRepo() 
{
    checkDuplicate(thisClass);
    thisClass = this;
    
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(spell/[a-z ]{2,})/(run[a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(affect/[a-z ]{2,})/([a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(skillcommand/[a-z ]{2,})/([a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(command/[a-z ]{2,})/([a-zA-Z]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(areas/[-a-z0-9_]{2,}.are/[a-z]+)/([-0-9.a-zA-Z_ ]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(areas/[-a-z0-9_]{2,}.are)/([a-z_ ]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^([a-z]+)/([-0-9.a-zA-Z_ :]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(global/on[A-Z][a-zA-Z]+)/([-_a-zA-Z ]+)$", true));
    subjPatterns.push_back(
        RegExp::Pointer(NEW, "^(global/post[A-Z][a-zA-Z]+)/([-_a-zA-Z ]+)$", true));
}

CodeSourceRepo::~CodeSourceRepo() 
{
    thisClass = 0;    
}

/** 
 * Check file name against allowed patterns and fill in corresponding sub-paths (folder and file name). 
 */
bool CodeSourceRepo::applySubjPatterns(const DLString &csNameConst, DLString &folderName, DLString &fileName, bool &isPublic) const
{
    DLString pubPrefix("public/");
    DLString csName(csNameConst);

    isPublic = false;

    // Scripts beginning with "public/" going to be published to dreamland_fenia_public repo
    if (pubPrefix.strPrefix(csName)) {
        // Strip "public/" part to check the rest of the pathname against the patterns
        csName.replace(0, pubPrefix.size(), "");
        isPublic = true;
    }

    for (auto &pattern: subjPatterns) {
        RegExp::MatchVector matches = pattern->subexpr(csName.c_str());
        if (!matches.empty()) {
            folderName = matches.at(0);
            fileName = matches.at(1);
            return true;
        }
    }

    return false;
}

void CodeSourceRepo::save(Scripting::CodeSource &cs) 
{
    DLString folderName;
    DLString fileName;
    bool isPublic;

    if (!applySubjPatterns(cs.name, folderName, fileName, isPublic)) {
        LogStream::sendWarning() << "CS repo: unknown pattern for " << cs.name << endl;
        return;
    }

    fs::path path = dreamland->getFeniaScriptDir().getAbsolutePath().c_str();

    if (isPublic) {
        // Re-add "public" folder part as folderName won't have it
        path /= "public";
    } 

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

bool CodeSourceRepo::readAll(const DLString &folderName)
{
    int success = 0, errors = 0;

    try {
        fs::path base = dreamland->getFeniaScriptDir().getAbsolutePath().c_str();
        fs::path path = base;
        if (!folderName.empty())
            path /= folderName.c_str();

        for (const auto& dirEntry : fs::recursive_directory_iterator(path)) {
            DLString stubFileName, stubFolderName;
            bool isPublic;

            if (!dirEntry.is_regular_file())
                continue;

            DLString csName = fs::relative(dirEntry.path(), base).generic_string();

            if (!applySubjPatterns(csName, stubFolderName, stubFileName, isPublic))
                continue;

            LogStream::sendNotice() << "CS repo: loading " << csName << endl;

            if (read(csName))
                success++;
            else
                errors++;
        }

        LogStream::sendNotice() << "CS repo: read " << success << " scenarios from path " << folderName << ", with " << errors << " errors." << endl;

    } catch (const std::exception &ex) {
        LogStream::sendError() << "CS repo: read all " << folderName << ": " << ex.what() << endl;
        return false;
    }

    return errors == 0;
}

bool CodeSourceRepo::read(const DLString &csName) 
{
    stringstream content;

    try {
        fs::path path = dreamland->getFeniaScriptDir().getAbsolutePath().c_str();
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

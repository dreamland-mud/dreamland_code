#ifndef __CODESOURCEREPO_H__
#define __CODESOURCEREPO_H__

#include <list>
#include "plugin.h"
#include "oneallocate.h"
#include "util/regexp.h"

namespace Scripting {
    class CodeSource;
}

class CodeSourceRepo: public Plugin, public OneAllocate {
public:
    CodeSourceRepo();
    virtual ~CodeSourceRepo();

    virtual void initialization() { }
    virtual void destruction() { }

    void save(Scripting::CodeSource &cs);
    void saveAll();
    bool read(const DLString &csName);
    bool readAll(const DLString &folderName);

    static CodeSourceRepo* getThis( ) { return thisClass; }

private:
    static CodeSourceRepo *thisClass;
    std::list<RegExp::Pointer> subjPatterns;
};

#endif // __CODESOURCEREPO_H__
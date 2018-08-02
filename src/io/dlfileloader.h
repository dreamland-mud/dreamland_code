/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DLFILELOADER_H
#define DLFILELOADER_H

#include <map>
#include "dlfileop.h"
#include "dldirectory.h"

class DLFileLoader {
public:    
    struct FileData {
        DLString content;
        time_t mtime;
    };
    typedef map<DLString, FileData> Files;
    
    virtual ~DLFileLoader();

    void loadAll();
    void load(const DLString &key);
    const DLString & get(const DLString &key) const;
    time_t getModifyTime(const DLString &key) const;
    
protected:
    virtual DLDirectory getTableDir() const = 0;
    virtual DLString getFileExt() const;

    void addEntry(const DLFile &entry);

    Files files;
};

class DLFileReaderByIndex {
public:
    virtual ~DLFileReaderByIndex();

    bool hasNext();
    DLFile next();

protected:
    virtual DLDirectory getTableDir() const = 0;
    virtual DLString getIndexName() const = 0;
    
    bool openIndex();
    void readEntry();
    DLFileRead fileIndex;
    DLString currentEntry;
};

#endif

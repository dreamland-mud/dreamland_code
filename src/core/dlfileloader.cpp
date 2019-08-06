/* $Id$
 *
 * ruffina, 2004
 */
#include "dlfileloader.h"
#include "dlfilestream.h"
#include "dreamland.h"
#include "logstream.h"

DLFileLoader::DLFileLoader(const DLString &tableDirName, const DLString &fileExt)
{
    this->fileExt = fileExt;
    this->tableDirName = tableDirName;
}

DLFileLoader::~DLFileLoader()
{
}

void DLFileLoader::loadAll()
{
    DLDirectory table(dreamland->getTableDir(), tableDirName);

    try {
        table.open();

        for (;;) {
            DLFile entry(table, table.nextTypedEntry(fileExt));
            addEntry(entry);
        }
    }
    catch (const ExceptionDBIO &ex) {
        LogStream::sendError() << "DLFileLoader: " << ex.what() << endl;
    }
    catch (const ExceptionDBIOEOF &eof) {
    }
}

void DLFileLoader::addEntry(const DLFile &entry)
{
    DLFileStream entryStream(entry);

    ostringstream buf;
    entryStream.toStream(buf);

    FileData fd;
    fd.content = buf.str();
    fd.mtime = entry.getModifyTime();

    files[entry.getFileName()] = fd;
}

void DLFileLoader::load(const DLString &key)
{
    try {
        DLFile entry(tableDirName, key, fileExt);
        addEntry(entry);
    }
    catch (const ExceptionDBIO &ex) {
        LogStream::sendError() << "DLFileLoader: " << ex.what() << endl;
    }
}

const DLString & DLFileLoader::get(const DLString &key) const
{
    Files::const_iterator t = files.find(key);

    if (t == files.end())
        return DLString::emptyString;

    return t->second.content;
}

const DLFileLoader::Files &DLFileLoader::getAll() const
{
    return files;
}
time_t DLFileLoader::getModifyTime(const DLString &key) const
{
    Files::const_iterator t = files.find(key);

    if (t == files.end())
        return 0;

    return t->second.mtime;
}

DLFileReaderByIndex::~DLFileReaderByIndex()
{
}

bool DLFileReaderByIndex::openIndex()
{
    if (fileIndex.getFP())
        return true;

    fileIndex = DLFileRead(getTableDir(), getIndexName());

    if (!fileIndex.open())
        return false;
    
    return true;
}

bool DLFileReaderByIndex::hasNext()
{
    if (!openIndex())
        return false;

    if (currentEntry == "$")
        return false;

    if (!currentEntry.empty())
        return true;
    
    readEntry();
    return hasNext();
}

void DLFileReaderByIndex::readEntry()
{
    char buf[256];

    if (!fileIndex.scanf("%s\n", buf))
        throw ExceptionDBIOEOF();

    currentEntry = buf;
}

DLFile DLFileReaderByIndex::next()
{
    if (!hasNext())
        return DLFile::emptyFile;
    
    DLString entry = currentEntry;

    currentEntry.clear();

    return DLFile(getTableDir(), entry);
}


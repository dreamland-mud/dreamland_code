#ifndef WEBNOTE_H
#define WEBNOTE_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmllong.h"
#include "xmllist.h"
#include "bitstring.h"

class Note;

// Note representation for the website.
class WebNote : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<WebNote> Pointer;
    
    WebNote();
    WebNote( const Note *, bool fColor );
    virtual ~WebNote( );

public:
    XML_VARIABLE XMLString subject;
    XML_VARIABLE XMLString from;
    XML_VARIABLE XMLString date;
    XML_VARIABLE XMLString text;
    XML_VARIABLE XMLLong id;
};

typedef bool (*NoteToInclude)(const Note *note);

// Represents list of notes for the web.
class WebNoteList : public XMLListBase<WebNote> {
public:
    WebNoteList();

    void importThread(const DLString &threadName, NoteToInclude func, bool fColor);
    void saveTo(const DLString &filePath);
};


#endif

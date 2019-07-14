#include "webnote.h"
#include "note.h"
#include "notethread.h"
#include "notemanager.h"
#include "xmlfile.h"
#include "mudtags.h"

WebNote::WebNote()
{

}

/**
 * Construct web representation of a note, with date formatted correctly.
 */
WebNote::WebNote( const Note *note, bool fColor ) 
{
    subject.setValue( note->getSubject( ).colourStrip( ) );
    from.setValue( note->getFrom( ).colourStrip( ) );
    date.setValue( note->getDate( ).getTimeAsString( "%d %b %Y") );
    id.setValue( note->getID( ) );

    // Strip {hl and so on. Color strip on demand.
    ostringstream txt;
    vistags_convert(note->getText().c_str(), txt);
    text.setValue(txt.str());
    if (!fColor)
        text.colourstrip(); 
}

WebNote::~WebNote( )
{
}

WebNoteList::WebNoteList()
             : XMLListBase(true)
{

}

/**
 * Import all notes of given type, filtered by predicate.
 */
void WebNoteList::importThread(const DLString &threadName, NoteToInclude predicate, bool fColor)
{
    NoteThread::NoteList::const_iterator i;
    NoteThread::Pointer thread = NoteManager::getThis( )->findThread(threadName);

    if (!thread)
        return;
    
    // Make sure timestamps are already in Russian when getTimeAsString is invoked.
    setlocale(LC_TIME, "ru_RU.KOI8-R");

    for (i = thread->getNoteList( ).begin( ); i != thread->getNoteList( ).end( ); i++) 
        if (predicate(*i))
            push_back( WebNote(*i, fColor) );
}

/**
 * Persist note list to disk.
 */
void WebNoteList::saveTo(const DLString &filePath)
{
    XMLFile file(filePath, NoteThread::NODE_NAME, this);
    file.save();
}


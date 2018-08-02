/* $Id: noteattrs.cpp,v 1.1.2.7.6.1 2007/06/26 07:18:19 rufina Exp $
 *
 * ruffina, 2004
 */

#include "noteattrs.h"
#include "note.h"
#include "notethread.h"

#include "class.h"
#include "so.h"

#include "pcharacter.h"
#include "dreamland.h"

/*-----------------------------------------------------------------------------
 * XMLAttributeLastRead
 *-----------------------------------------------------------------------------*/
XMLAttributeLastRead::XMLAttributeLastRead( ) 
{
}

time_t XMLAttributeLastRead::getStamp( const NoteThread * thread ) const
{
    XMLMapBase<XMLLong>::const_iterator i;

    i = stamps.find( thread->getName( ) );

    if (i == stamps.end( ))
	return 0;
    else
	return i->second.getValue( );
}

void XMLAttributeLastRead::updateStamp( const NoteThread * thread )
{
    stamps[thread->getName( )] = dreamland->getCurrentTime( );
}

void XMLAttributeLastRead::setStamp( const NoteThread * thread, time_t stamp )
{
    stamps[thread->getName( )] = stamp;
}

void XMLAttributeLastRead::updateStamp( const NoteThread * thread, const Note* note )
{
    XMLMapBase<XMLLong>::iterator i;

    i = stamps.find( thread->getName( ) );

    if (i == stamps.end( ))
	stamps[thread->getName( )] = note->getID( );
    else
	stamps[thread->getName( )] = std::max( note->getID( ), (time_t)i->second.getValue( ) );
}

/*-----------------------------------------------------------------------------
 * XMLNoteData
 *-----------------------------------------------------------------------------*/

XMLNoteData::XMLNoteData( ) 
{
}

void XMLNoteData::addLine( const DLString &arg )
{
    body.push_back( arg );
}
void XMLNoteData::delLine( )
{
    body.pop_back( );
}
void XMLNoteData::clearBody( )
{
    body.clear( );
}
bool XMLNoteData::isBodyEmpty( ) const
{
    return body.empty( );
}
int XMLNoteData::getBodySize( ) const
{
    int size = 0;
    Lines::const_iterator i; 

    for (i = body.begin( ); i != body.end( ); i++)
	size += i->getValue( ).size( ); 

    return size;
}

void XMLNoteData::linesToStream( ostringstream& buf ) const
{
    for (Lines::const_iterator i = body.begin( ); i != body.end( ); i++)
	buf << *i << endl;
}
void XMLNoteData::toStream( ostringstream& buf ) const
{
    buf << getFrom( ) << "{x: "
	<< getSubject( ) << "{x" << endl
	<< "To: " << getRecipient( ) << "{x" << endl;
    linesToStream( buf );
}
void XMLNoteData::commit( Note *note ) const
{
    ostringstream buf;

    linesToStream( buf );

    note->setText( buf.str( ) );
    note->setSubject( getSubject( ) );
    note->setRecipient( getRecipient( ) );
    note->setAuthor( getAuthor( ) );
    note->setDate( );

    if (!from.getValue( ).empty( ))
	note->setFrom( getFrom( ) );
}

/*-----------------------------------------------------------------------------
 * XMLAttributeNoteData
 *-----------------------------------------------------------------------------*/
const DLString XMLAttributeNoteData::TYPE = "XMLAttributeNoteData";

void XMLAttributeNoteData::clearNote( const NoteThread *thread )
{
    iterator i = find( thread->getName( ) );

    if (i != end( ))
	erase( i );
}


XMLNoteData * XMLAttributeNoteData::findNote( const NoteThread *thread )
{
    iterator i = find( thread->getName( ) );

    if (i != end( ))
	return &i->second;
    else
	return NULL;
}

XMLNoteData * XMLAttributeNoteData::makeNote( PCharacter *ch, const NoteThread *thread )
{
    XMLNoteData *note;
    
    note = &(*this)[thread->getName( )];
    note->setThreadName( thread->getName( ) );
    note->setAuthor( ch->getName( ) );
    return note;
}

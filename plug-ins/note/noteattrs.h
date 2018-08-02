/* $Id: noteattrs.h,v 1.1.2.10 2006/01/03 09:50:53 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef NOTEATTRS_H
#define NOTEATTRS_H

#include <sstream>

#include "xmlmap.h"
#include "xmllist.h"
#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmllong.h"

#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "playerattributes.h"

class Note;
class NoteThread;
class PCharacter;

class XMLAttributeLastRead : public RemortAttribute, 
			     public XMLVariableContainer 
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeLastRead> Pointer;

	XMLAttributeLastRead( );
	
	time_t getStamp( const NoteThread * ) const;
	void setStamp( const NoteThread *, time_t );
	void updateStamp( const NoteThread * );
	void updateStamp( const NoteThread *, const Note * );

private:
	XML_VARIABLE XMLMapBase<XMLLong> stamps;
};

class XMLNoteData : public XMLVariableContainer {
XML_OBJECT
public:
	typedef XMLListBase<XMLString> Lines;

	XMLNoteData( );
	
	inline const DLString& getThreadName( ) const;
	inline void setThreadName( const DLString & );

	inline void setRecipient( const DLString & );
	inline const DLString& getRecipient( ) const;
	
	inline void setFrom( const DLString & );
	inline const DLString& getFrom( ) const;
	
	inline const DLString & getAuthor( ) const;
	inline void setAuthor( const DLString & );

	inline void setSubject( const DLString & );
	inline const DLString& getSubject( ) const;

	void addLine( const DLString& );
	void delLine( );
	void clearBody( );
	bool isBodyEmpty( ) const;
	int getBodySize( ) const;
	
	void toStream( ostringstream& ) const;
        void linesToStream( ostringstream & ) const;
	void commit( Note * ) const;

private:
	XML_VARIABLE XMLString threadName;
	XML_VARIABLE XMLString recipient;
	XML_VARIABLE XMLString from;
	XML_VARIABLE XMLString author;
	XML_VARIABLE XMLString subject;
	XML_VARIABLE Lines body;
};

class XMLAttributeNoteData: public XMLAttribute, public XMLMapBase<XMLNoteData> {
public:
	typedef ::Pointer<XMLAttributeNoteData> Pointer;

	virtual const DLString & getType( ) const
	{
	    return TYPE;
	}

	static const DLString TYPE;

	void clearNote( const NoteThread * );
	XMLNoteData * findNote( const NoteThread * );
	XMLNoteData * makeNote( PCharacter *, const NoteThread * );
};

inline const DLString& XMLNoteData::getThreadName( ) const
{
    return threadName.getValue( );
}
inline void XMLNoteData::setThreadName( const DLString &arg )
{
    threadName.setValue( arg );
}
inline void XMLNoteData::setRecipient( const DLString &arg )
{
    recipient.setValue( arg );
}
inline const DLString& XMLNoteData::getRecipient( ) const
{
    return recipient.getValue( );
}
inline void XMLNoteData::setFrom( const DLString &arg )
{
    from.setValue( arg );
}
inline const DLString & XMLNoteData::getFrom( ) const
{
    return (from.getValue( ).empty( ) ? getAuthor( ) : from.getValue( ));
}
inline void XMLNoteData::setSubject( const DLString &arg )
{
    subject.setValue( arg );
}
inline const DLString& XMLNoteData::getSubject( ) const
{
    return subject.getValue( );
}
inline void XMLNoteData::setAuthor( const DLString & value )
{
    author.setValue( value );
}
inline const DLString & XMLNoteData::getAuthor( ) const
{
    return author.getValue( );
}

#endif


/* $Id: note.h,v 1.1.2.8.6.4 2009/11/08 17:41:56 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __NOTE_H__
#define __NOTE_H__

#include <sstream>

#include "xmlvariablecontainer.h"
#include "xmldate.h"
#include "xmlstring.h"
#include "xmllong.h"
#include "xmlboolean.h"
#include "xmlflags.h"

#include "dreamland.h"

class PCharacter;
class PCMemoryInterface;
class Clan;
class Profession;
class Race;

class Note : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<Note> Pointer;
    
    Note( );
    virtual ~Note( );

    inline const DLString & getSubject( ) const;
    inline const DLString & getRecipient( ) const;
    inline const DLString & getFrom( ) const;
    inline const DLString & getAuthor( ) const;
    inline const DLString & getText( ) const;
    inline const Flags & getFlags() const;
    inline time_t getID( ) const;
    inline const Date getDate( ) const;

    inline void setSubject( const DLString & );
    inline void setRecipient( const DLString & );
    inline void setFrom( const DLString & );
    inline void setAuthor( const DLString & );
    inline void setText( const DLString & );
    inline void setFlags( const bitstring_t & );
    inline void setDate( );
    inline void setDate( int );
    
    void toStream( int, ostringstream & ) const;
    void toForwardStream( ostringstream & ) const;
    
    inline bool operator < (const Note & ) const;

    bool isNoteTo( PCMemoryInterface * ) const;
    bool isNoteFrom( PCMemoryInterface * ) const;
    bool isNoteToAll( ) const;
    static bool parseRecipient( PCharacter *, const DLString &, ostringstream & );
    
private:
    static bool findRecipient( PCMemoryInterface *, DLString &, ostringstream & );
    static Profession * findProf( const DLString & );
    static Clan * findClan( const DLString & );
    static Race * findRace( const DLString & );

    XML_VARIABLE XMLString subject;
    XML_VARIABLE XMLString recipient;
    XML_VARIABLE XMLString from;
    XML_VARIABLE XMLString author;
    XML_VARIABLE XMLString text;
    XML_VARIABLE XMLLong id;
    XML_VARIABLE XMLFlagsNoEmpty flags;
public:    
    XML_VARIABLE XMLBooleanNoFalse godsSeeAlways;
};

inline const DLString & Note::getSubject( ) const
{
    return subject.getValue( );
}
inline const DLString & Note::getRecipient( ) const
{
    return recipient.getValue( );
}
inline const DLString & Note::getFrom( ) const
{
    return (from.getValue( ).empty( ) ? getAuthor( ) : from.getValue( ));
}
inline const DLString & Note::getAuthor( ) const
{
    return author.getValue( );
}
inline const DLString & Note::getText( ) const
{
    return text.getValue( );
}
inline time_t Note::getID( ) const
{
    return id.getValue( );
}
inline const Date Note::getDate( ) const
{
    return id.getValue( );
}
inline const Flags & Note::getFlags() const
{
    return flags;
}

inline void Note::setSubject( const DLString & value )
{
    subject.setValue( value );
}
inline void Note::setRecipient( const DLString & value )
{
    recipient.setValue( value );
}
inline void Note::setFrom( const DLString & value )
{
    from.setValue( value );
}
inline void Note::setAuthor( const DLString & value )
{
    author.setValue( value );
}
inline void Note::setText( const DLString & value )
{
    text.setValue( value );
}
inline void Note::setFlags( const bitstring_t &value )
{
    flags.setValue(value);
}
inline void Note::setDate( )
{
    id = dreamland->getCurrentTime( );
}
inline void Note::setDate( int value )
{
    id = value;
}

inline bool Note::operator < ( const Note &note ) const
{
    return getID( ) < note.getID( );
}


#endif

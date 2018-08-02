/* $Id$
 *
 * ruffina, 2004
 */
#include "replay.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "arg_utils.h"

static const DLString ATTR_PRIVATE = "history_private";
static const DLString ATTR_NEAR = "history_near";
static const DLString ATTR_PUBLIC = "history_public";
static const DLString ATTR_ALL = "history_all";
static const DLString ATTR_TELLS = "tells";

const int MAX_HISTORY_SIZE = 50;
const int DEFAULT_REPLAY_SIZE = 10;

/**
 * говорит тебе 'сто'
 * говорит тебе 'девяносто девять'
 * ...
 * говорит тебе 'девяносто один'         <------- DEFAULT_REPLAY_SIZE cuf off
 * ...
 * говорит тебе 'три'
 * говорит тебе 'два'
 * говорит тебе 'раз'                    <------- MAX_HISTORY_SIZE cut off
 * говорит тебе 'начинаю считать'
 * 
 */
static bool replay_messages( ostringstream &buf, PCharacter *ch, const DLString &attrName, int limit )
{
    XMLStringListAttribute::Pointer attr  
		= ch->getAttributes( ).findAttr<XMLStringListAttribute>( attrName );
    if (!attr)
        return false;

    XMLStringListAttribute::iterator a;
    list<DLString> messages;
    int i;
    for (a = attr->begin( ), i = 0; a != attr->end( ) && i < limit; a++, i++) 
        messages.push_front( *a );

    if (messages.empty( ))
        return false;

    for (list<DLString>::iterator m = messages.begin( ); m != messages.end( ); m++)
        buf << " ** " << *m << endl;

    buf << endl;
    return true;
}


bool replay_history_all( ostringstream &buf, PCharacter *ch, int limit = MAX_HISTORY_SIZE )
{
    return replay_messages( buf, ch, ATTR_ALL, limit );
}

bool replay_history_private( ostringstream &buf, PCharacter *ch, int limit = DEFAULT_REPLAY_SIZE )
{
    return replay_messages( buf, ch, ATTR_PRIVATE, limit );
}

bool replay_history_public( ostringstream &buf, PCharacter *ch, int limit = DEFAULT_REPLAY_SIZE )
{
    return replay_messages( buf, ch, ATTR_PUBLIC, limit );
}

bool replay_history_near( ostringstream &buf, PCharacter *ch, int limit = DEFAULT_REPLAY_SIZE )
{
    return replay_messages( buf, ch, ATTR_NEAR, limit );
}

static void remember_one_message( PCharacter *ch, const DLString &msg, const DLString &attrName )
{
    XMLStringListAttribute::Pointer attr  
		= ch->getAttributes( ).getAttr<XMLStringListAttribute>( attrName );

    attr->push_front( msg );
    if (attr->size( ) > MAX_HISTORY_SIZE)
        attr->resize( MAX_HISTORY_SIZE );
}

void remember_history_public( PCharacter *ch, const DLString &msg )
{
    remember_one_message( ch, msg, ATTR_PUBLIC );
    remember_one_message( ch, msg, ATTR_ALL );
}

void remember_history_private( PCharacter *ch, const DLString &msg )
{
    remember_one_message( ch, msg, ATTR_PRIVATE );
    remember_one_message( ch, msg, ATTR_ALL );
}

void remember_history_near( PCharacter *ch, const DLString &msg )
{
    remember_one_message( ch, msg, ATTR_NEAR );
    remember_one_message( ch, msg, ATTR_ALL );
}



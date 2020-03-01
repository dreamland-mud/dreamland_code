/* $Id$
 *
 * ruffina, 2004
 */
#include "replay.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "arg_utils.h"

static const DLString ATTR_PRIVATE = "history_private";
static const DLString ATTR_NEAR = "history_near";
static const DLString ATTR_PUBLIC = "history_public";
static const DLString ATTR_ALL = "history_all";

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


/** 
 * Called from stop_fighting. 
 * Notify about missed messages, but not too often - remember last time. 
 */
bool ReplayAttribute::handle( const StopFightArguments &args )
{                
    if (tells.empty())
        return true;

    long long now = dreamland->getCurrentTime();

    if (now - lastNotified > Date::SECOND_IN_MINUTE) {
        notify(args.pch);
        lastNotified = now;
    }

    return true;
}

/** Called from 'afk' command. Notify about missed messages. */
bool ReplayAttribute::handle( const AfkArguments &args )
{
    if (args.on)
        return true;

    notify(args.pch);
    return true;
}

void ReplayAttribute::notify(PCharacter *ch) const
{
    if (tells.size() > 0)
        ch->pecho(
            "Тебя ожидает {R%1$d{x сообщен%1$Iие|ия|ий, используй команду {hc{y{lRпрослушать{lEreplay{x для просмотра.",
            tells.size());    
}

void ReplayAttribute::addMessage(const DLString &msg)
{
    tells.push_back(msg);
    lastNotified = 0;
}

/** Replay stored messages and wipe out the attribute. */
bool ReplayAttribute::playAndErase(ostringstream &buf, PCharacter *ch)
{
    ReplayAttribute::Pointer replay
                = ch->getAttributes().findAttr<ReplayAttribute>( "replay" );
    
    if (!replay)
        return false;

    for (auto &t: replay->tells)
        buf << " ** " << t << endl;
    
    bool rc = replay->tells.size() > 0;
    ch->getAttributes( ).eraseAttribute( "replay" );
    return rc;
}

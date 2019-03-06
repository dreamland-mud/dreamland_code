/* $Id$
 *
 * ruffina, 2004
 */
#include "json/json.h"
#include "notehooks.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "dreamland.h"

NoteHooks *noteHooks = NULL;

NoteHooks::NoteHooks( )
{
    noteHooks = this;
}

NoteHooks::~NoteHooks( )
{
    noteHooks = NULL;
}

void NoteHooks::initialization( )
{
}

void NoteHooks::destruction( )
{
}    

static string json_to_string( const Json::Value &value )
{
    Json::FastWriter writer;
    return writer.write( value );
}    

static const DLString DISCORD_USERNAME = "Новости мира DreamLand";
static const DLString DISCORD_FOLDER = "discord";
static const DLString SEPARATOR = "\n--------------------------------------------------------------------------------\n";

void NoteHooks::processNoteMessage( const NoteThread &thread, const Note &note ) const
{
    if (!note.isNoteToAll( ))
        return;

    // TODO: make supported thread names and hook types configurable.
    if (thread.getName( ) != "news" && thread.getName( ) != "change" && thread.getName() != "qnote")
        return;

    ostringstream content;
    content << "**Тема: " << note.getSubject( ).colourStrip( ) << "**" << endl 
            << note.getText( ).colourStrip( ) << SEPARATOR;

    // Compose JSON message for Discord webhook.
    Json::Value body;
    body["content"] = content.str( );
    body["username"] = DISCORD_USERNAME;
    DLString message = json_to_string( body );
    LogStream::sendNotice( ) << "Note hook for " << thread.getName( ) << " " << note.getID( ) << ": " << message << endl;

    // Create temporary file in a subfolder, for Discord sync job to pick up.
    DLDirectory dir( dreamland->getMiscDir( ), DISCORD_FOLDER );
    DLFileStream( dir.tempEntry( ) ).fromString( message );
}


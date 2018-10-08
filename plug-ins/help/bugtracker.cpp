/* $Id$
 *
 * ruffina, 2004
 */
#include "bugtracker.h"
#include "logstream.h"
#include "date.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "character.h"
#include "room.h"
#include "dreamland.h"

/*
 * Bug status: U(ndefined) F(ixed) R(ewarded) P(ostponed) C(ancelled) D(uplicate)
 */
BugTracker *bugTracker = NULL;

BugTracker::BugTracker( )
{
    checkDuplicate( bugTracker );
    bugTracker = this;
}

BugTracker::~BugTracker( )
{
    bugTracker = NULL;
}

void BugTracker::reportBug( Character *ch, const DLString &txt ) const
{
    writeFile( ch, bugFile, bugDir, txt );
}

void BugTracker::reportTypo( Character *ch, const DLString &txt ) 
{
    writeFile( ch, typoFile, typoDir, txt );
}

void BugTracker::reportIdea( Character *ch, const DLString &txt ) const
{
    writeFile( ch, ideaFile, ideaDir, txt );
}

void BugTracker::reportNohelp( Character *ch, const DLString &txt ) const
{
    writeFile( ch, nohelpFile, nohelpDir, txt );
}

void BugTracker::writeFile( Character *ch, const DLString &filename, const DLString &dirname, const DLString &txt ) const
{
    // Compose message for trello checklist.
    DLString message;
    message << "[" << ch->in_room->vnum  << "] " << ch->getNameP( ) << ": " << txt;

    // Create temporary file in a subfolder, for trello sync job to pick up.
    DLDirectory dir( dreamland->getMiscDir( ), dirname );
    DLFileStream( dir.tempEntry( ) ).fromString( message );
    
    // Log typos normally.
    DLFileAppend( dreamland->getMiscDir( ), filename ).printf(
        "[T][   %2s][%5d][%8s] %s: %s\n", 
        ch->is_immortal( ) ? "  " : "qp",
        ch->in_room->vnum, 
        Date::getCurrentTimeAsString( "%d/%m/%y" ).c_str( ),
        ch->getNameP( ), 
        txt.c_str( ) );
}


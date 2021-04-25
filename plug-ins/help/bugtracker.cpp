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

void BugTracker::reportMessage(const DLString &msgType, Character *ch, const DLString &message) const
{
    reportMessage(msgType, ch->getName(), message, ch->in_room->vnum);

}

void BugTracker::reportNohelp( Character *ch, const DLString &txt ) const
{
    reportMessage("nohelp", ch, txt);
}

const DLString &BugTracker::getDirName(const DLString &msgType) const
{
    // TODO: a bit less yuck.
    if (msgType == "idea") return ideaDir;
    if (msgType == "typo") return typoDir;
    if (msgType == "nohelp") return nohelpDir;
    if (msgType == "bug") return bugDir;
    return DLString::emptyString;
}

const DLString &BugTracker::getFileName(const DLString &msgType) const
{
    if (msgType == "idea") return ideaFile;
    if (msgType == "typo") return typoFile;
    if (msgType == "nohelp") return nohelpFile;
    if (msgType == "bug") return bugFile;
    return DLString::emptyString;
}

void BugTracker::reportMessage(const DLString &msgType, const DLString &authorName, const DLString &message, const DLString &location) const
{
    DLString filename = getFileName(msgType);
    DLString dirname = getDirName(msgType);
    if (filename.empty() || dirname.empty())
        return;

    // Compose message for trello checklist.
    DLString mbuf;
    if (!location.empty())
        mbuf << "[" << location << "] ";
    mbuf << authorName << ": " << message;

    // Create temporary file in a subfolder, for trello sync job to pick up.
    DLDirectory dir( dreamland->getMiscDir( ), dirname );
    DLFileStream( dir.tempEntry( ) ).fromString( mbuf );
    
    // Log typos normally.
    DLFileAppend( dreamland->getMiscDir( ), filename ).printf(
        "[%s][%s] %s: %s\n", 
        location.c_str(), 
        Date::getCurrentTimeAsString( "%d/%m/%y" ).c_str( ),
        authorName.c_str(), 
        message.c_str());
}

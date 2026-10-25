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
#include "pcharacter.h"
#include "room.h"
#include "dreamland.h"

#include <cctype>

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

void BugTracker::reportMessage(const DLString &msgType, PCharacter *ch, const DLString &message) const
{
    reportMessage(msgType, ch->getName(), message, ch->in_room->vnum);

}

void BugTracker::reportNohelp( PCharacter *ch, const DLString &txt ) const
{
    writeMessage("nohelp", ch->getName(), txt, ch->in_room->vnum, nohelpWorthTrello(ch, txt));
}

void BugTracker::reportNohelpAnswered( PCharacter *ch, const DLString &txt ) const
{
    writeMessage("nohelp", ch->getName(), "[fz] " + txt, ch->in_room->vnum, false);
}

// nohelp.txt keeps every miss for analysis, the Trello feed only gets what a human should read:
// no immortals or API bots, no bare numbers or 1-2 char stubs, one copy of a query per 24 hours.
bool BugTracker::nohelpWorthTrello(PCharacter *ch, const DLString &txt) const
{
    static const time_t window = 24 * 60 * 60;

    if (ch->is_immortal() || ch->getName() == "ExecBot" || ch->getName() == "TestBot")
        return false;

    DLString query = txt;
    query.stripWhiteSpace();
    if (query.size() < 3)
        return false;

    bool digitsOnly = true;
    for (DLString::size_type i = 0; i < query.size(); i++)
        if (!isdigit((unsigned char)query[i]) && query[i] != '.' && query[i] != ' ') {
            digitsOnly = false;
            break;
        }
    if (digitsOnly)
        return false;

    time_t now = dreamland->getCurrentTime();
    for (auto it = nohelpSeen.begin(); it != nohelpSeen.end(); )
        if (now - it->second >= window)
            it = nohelpSeen.erase(it);
        else
            ++it;

    DLString key = query.toLower();
    if (nohelpSeen.count(key))
        return false;

    nohelpSeen[key] = now;
    return true;
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
    writeMessage(msgType, authorName, message, location, true);
}

void BugTracker::writeMessage(const DLString &msgType, const DLString &authorName, const DLString &message, const DLString &location, bool toTrello) const
{
    DLString filename = getFileName(msgType);
    DLString dirname = getDirName(msgType);
    if (filename.empty() || dirname.empty())
        return;

    // Compose message for trello checklist.
    DLString mbuf;
    if (!location.empty())
        mbuf << "[" << location << "]";

    mbuf << "[" << Date::getCurrentTimeAsString("%Y/%m/%d %H:%M") << "] ";
    mbuf << authorName << ": " << message;

    // Create temporary file in a subfolder, for trello sync job to pick up.
    if (toTrello) {
        DLDirectory dir( dreamland->getMiscDir( ), dirname );
        DLFileStream( dir.tempEntry( ) ).fromString( mbuf );
    }
    
    // Log typos normally.
    DLFileAppend( dreamland->getMiscDir( ), filename ).printf(
        "[%s][%s] %s: %s\n", 
        location.c_str(), 
        Date::getCurrentTimeAsString( "%d/%m/%y" ).c_str( ),
        authorName.c_str(), 
        message.c_str());
}

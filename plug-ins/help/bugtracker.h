/* $Id$
 *
 * ruffina, 2004
 */
#ifndef BUGTRACKER_H
#define BUGTRACKER_H

#include "oneallocate.h"
#include "xmlconfigurableplugin.h"
#include "xmlvariablecontainer.h"
#include "xmlstring.h"

#include <map>

class PCharacter;

class BugTracker : public OneAllocate, 
                   public XMLConfigurablePlugin, 
                   public virtual XMLVariableContainer 
{
XML_OBJECT    
public:
    BugTracker( );
    virtual ~BugTracker( );
    
    void reportNohelp( PCharacter *ch, const DLString &txt ) const;
    /** A miss some fallback (a guess, a numbered list) answered: logged to
     *  the nohelp file with an [fz] tag for analysis, never queued for Trello. */
    void reportNohelpAnswered( PCharacter *ch, const DLString &txt ) const;
    void reportMessage(const DLString &msgType, PCharacter *ch, const DLString &message) const;
    void reportMessage(const DLString &msgType, const DLString &authorName, const DLString &message, const DLString &location = DLString::emptyString) const;

protected:
    const DLString &getDirName(const DLString &msgType) const;
    const DLString &getFileName(const DLString &msgType) const;
    bool nohelpWorthTrello(PCharacter *ch, const DLString &txt) const;
    void writeMessage(const DLString &msgType, const DLString &authorName, const DLString &message, const DLString &location, bool toTrello) const;

    XML_VARIABLE XMLString bugFile;
    XML_VARIABLE XMLString ideaFile;
    XML_VARIABLE XMLString typoFile;
    XML_VARIABLE XMLString nohelpFile;
    XML_VARIABLE XMLString typoDir;
    XML_VARIABLE XMLString nohelpDir;
    XML_VARIABLE XMLString ideaDir;
    XML_VARIABLE XMLString bugDir;

    // Lowercased nohelp query -> last time it was queued for Trello.
    mutable std::map<DLString, time_t> nohelpSeen;
};

extern BugTracker *bugTracker;

#endif

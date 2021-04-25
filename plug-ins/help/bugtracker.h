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

class Character;

class BugTracker : public OneAllocate, 
                   public XMLConfigurablePlugin, 
                   public virtual XMLVariableContainer 
{
XML_OBJECT    
public:
    BugTracker( );
    virtual ~BugTracker( );
    
    void reportNohelp( Character *ch, const DLString &txt ) const;
    void reportMessage(const DLString &msgType, Character *ch, const DLString &message) const;
    void reportMessage(const DLString &msgType, const DLString &authorName, const DLString &message, const DLString &location = DLString::emptyString) const;

protected:
    const DLString &getDirName(const DLString &msgType) const;
    const DLString &getFileName(const DLString &msgType) const;

    XML_VARIABLE XMLString bugFile;
    XML_VARIABLE XMLString ideaFile;
    XML_VARIABLE XMLString typoFile;
    XML_VARIABLE XMLString nohelpFile;
    XML_VARIABLE XMLString typoDir;
    XML_VARIABLE XMLString nohelpDir;
    XML_VARIABLE XMLString ideaDir;
    XML_VARIABLE XMLString bugDir;
};

extern BugTracker *bugTracker;

#endif

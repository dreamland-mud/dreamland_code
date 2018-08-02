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
    
    void reportBug( Character *, const DLString & ) const;
    void reportTypo( Character *, const DLString & );
    void reportIdea( Character *, const DLString & ) const;
    void reportNohelp( Character *, const DLString & ) const;

    void writeFile( Character *, const DLString &, const DLString &, const DLString & ) const;

protected:
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

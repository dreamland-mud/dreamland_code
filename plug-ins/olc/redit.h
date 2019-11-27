/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __REDIT_H__
#define __REDIT_H__

#include "olcstate.h"
#include "xmlindexdata.h"

class OLCStateRoom : public OLCStateTemplate<OLCStateRoom>,
                     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateRoom> Pointer;

    virtual void commit( );
    virtual void changed( PCharacter * );

    void attach(PCharacter *ch, Room *pRoom);
    virtual void detach(PCharacter *ch);

    template <typename T>
    bool cmd( PCharacter *, char * );

    bool change_exit(PCharacter *, char *, int);
    static Room *redit_create(PCharacter *, char *);
    static void show(PCharacter *ch, Room *, bool showWeb);

    XML_VARIABLE XMLInteger originalRoom;

private:
    virtual void statePrompt( Descriptor * );
};

#define REDIT(Cmd, rname, help) OLC_CMD(OLCStateRoom, Cmd, rname, help)

#endif

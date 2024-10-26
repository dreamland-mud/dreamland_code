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

    void attach(PCharacter *ch, RoomIndexData *pRoom);
    virtual void detach(PCharacter *ch);

    template <typename T>
    bool cmd( PCharacter *, char * );

    bool change_exit(PCharacter *, const char *, int);
    static RoomIndexData *redit_create(PCharacter *, char *);
    static void show(PCharacter *ch, RoomIndexData *, bool showWeb);

    XML_VARIABLE XMLInteger originalRoom;
    XML_VARIABLE XMLInteger room;

private:
    virtual void statePrompt( Descriptor * );

    RoomIndexData *getOriginal();
    void default_door_names(PCharacter *, int);
    static void create_exit(RoomIndexData *sourceIndex, int door, RoomIndexData *destIndex);
    static void delete_exit(RoomIndexData *pRoom, int door);
};

#define REDIT(Cmd, rname, help) OLC_CMD(OLCStateRoom, Cmd, rname, help)

#endif

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
    virtual DLString getJournalKind( ) const;
    virtual int getJournalVnum( ) const;

    void attach(PCharacter *ch, RoomIndexData *pRoom);
    virtual void detach(PCharacter *ch);

    template <typename T>
    bool cmd( PCharacter *, char * );

    bool change_exit(PCharacter *, const char *, int);
    static RoomIndexData *redit_create(PCharacter *, char *);
    /** An empty room under a vnum olc_create_vnum has cleared. */
    static RoomIndexData *createRoom(int vnum);
    /** Put a new prototype into the room maps and create its instance. */
    static void registerRoom(RoomIndexData *);
    /** Why the room can't be deleted now, or an empty string. */
    static DLString deleteRefusal(RoomIndexData *);
    /** Remove the room from every vnum lookup and mark its area changed. */
    static void unregisterRoom(RoomIndexData *);
    static void show(PCharacter *ch, RoomIndexData *, bool showWeb);
    static void move(PCharacter *ch, RoomIndexData *roomToMove, AreaIndexData *targetArea);
    /** The field groups of 'copy': names, descriptions, smell, sound, EDs;
     *  sector, flags, rates, clan, guilds, liquid; behaviors and props. */
    static void copyDescriptions(RoomIndexData *pRoom, RoomIndexData *source);
    static void copyParameters(RoomIndexData *pRoom, RoomIndexData *source);
    static void copyBehaviors(RoomIndexData *pRoom, RoomIndexData *source);
    /** Every saved field of fresh (an unregistered XMLRoom::compat result)
     *  onto pRoom in place, its instance synced. Deletes fresh. */
    static void replaceRoom(RoomIndexData *pRoom, RoomIndexData *fresh);
    /** Point the instance's exits at the prototype's targets and recompute
     *  its flags (prototype, room affects, NO_MOB when nothing leads out). */
    static void resolveInstance(RoomIndexData *pRoom);

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

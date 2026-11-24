#ifndef __ROOMTEMPDESCR_H__
#define __ROOMTEMPDESCR_H__

#include "extradescription.h"
#include "xmlmultistring.h"

class Room;

/*
 * Runtime-only extra descriptions on a room instance: graffiti, notes and
 * other things scripts put on a room for a while. They answer 'look', 'read'
 * and 'examine' after the room's own extra descriptions, and only to whole
 * words of their keyword. They live only in memory and are never written to
 * the area file. Their owner (Fenia) keeps its own store
 * and puts them back after a reboot or a reload of this plugin.
 *
 * Each one may carry a mark: a line shown under the room description while
 * it exists ("Somebody has left (doodles) on the wall here.").
 */
namespace RoomTempDescr {
    /** Temporary extra descriptions of the room, an empty list if none. */
    const ExtraDescrList & list( Room *room );

    /** Find one by its exact keyword string, or null. */
    ExtraDescription * find( Room *room, const DLString &keyword );

    /** Add or replace the one with this exact keyword string. An empty mark
     *  means no line under the room description. */
    void set( Room *room, const DLString &keyword,
              const XMLMultiString &text, const XMLMultiString &mark );

    /** Remove the one with this exact keyword string. True if it existed. */
    bool clear( Room *room, const DLString &keyword );

    /** Mark lines of all temporary descriptions in the room, in the given
     *  language, one per line with a trailing newline each. Empty if none. */
    DLString marks( Room *room, lang_t lang );
}

#endif

#ifndef EXITS_H
#define EXITS_H

#include "xmlmultistring.h"

class Room;
struct extra_exit_data;
struct exit_data;

typedef struct exit_data EXIT_DATA;
typedef struct extra_exit_data EXTRA_EXIT_DATA;

/*
 * Exit data.
 */
struct exit_data
{
    union
    {
        Room* to_room;
        int vnum;
    } u1;

    int exit_info;
    int exit_info_default;
    int  key;

    XMLMultiString keyword;
    XMLMultiString short_descr;
    XMLMultiString description;

    EXIT_DATA* next;
    int orig_door;
    int level;

    /** Resolve u1 from a virtual number to the real room. */
    void resolve();

    /** Restore exit flags to their original values. */
    void reset();

    exit_data* create(); // Implemented in loadsave plugin.
};

struct extra_exit_data
{
    extra_exit_data();
    virtual ~extra_exit_data();

    union
    {
        Room* to_room;
        int vnum;
    } u1;

    int exit_info;
    int exit_info_default;
    int key;
    int max_size_pass;


    XMLMultiString keyword;
    XMLMultiString short_desc_from;
    XMLMultiString short_desc_to;
    XMLMultiString description;
    XMLMultiString room_description;

    int level;

    XMLMultiString msgLeaveRoom;
    XMLMultiString msgLeaveSelf;
    XMLMultiString msgEntryRoom;
    XMLMultiString msgEntrySelf;

    /** Resolve u1 from a virtual number to the real room. */
    void resolve();

    /** Restore exit flags to their original values. */
    void reset();

    extra_exit_data* create(); // Implemented in loadsave plugin.
};


struct ExtraExitList : public list<extra_exit_data*> {
    /** Return an exit that matches the given keyword. */
    extra_exit_data* find(const DLString& keyword) const;

    /**
     * Remove matching exit from list and free its memory.
     * Returns true if found.
     */
    bool findAndDestroy(const DLString& keyword);
};


#endif

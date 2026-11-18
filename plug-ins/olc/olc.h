// $Id: olc.h,v 1.1.2.14.6.4 2010-09-01 21:20:46 rufina Exp $

// The version info.  Please use this info when reporting bugs.
// It is displayed in the game by typing 'version' while editing.
// Do not remove these from the code - by request of Jason Dinkel
#ifndef __OLC_H__
#define __OLC_H__

#include <list>
#include <set>
#include "onlinecreation.h"
#include "descriptor.h"
#include "multimessage.h"

#define OLC_VERSION "ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
                "     Port a ROM 2.4 v1.00\n\r"
#define OLC_AUTHOR  "     By Jason(jdinkelmines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbiifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4v4\n\r"     \
                "     Por Birdie (itoledoramses.centic.utem.cl)\n\r"
#define OLC_DATE    "     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
                "     (Port a ROM 2.4 - Nov 2, 1996)\n\r"
#define OLC_CREDITS "     Original by Surreality(cxw197psu.edu) and Locke(lockelm.com)"

bool show_help(Character * ch, const char *argument);
int help_next_free_id();
DLString show_enum_array(const EnumerationArray &array);
DLString show_enum_array_web(const EnumerationArray &array);
void show_behaviors(PCharacter *ch, const GlobalBitvector &behaviors, const Json::Value &props);

#define stc(t, c) (c)->send_to((t))

const char * get_skill_name( int sn, bool verbose = true );
void ptc(Character *c, const char *fmt, ...);
/* Trilinguality (Trello 2594, Phase 4): resolve for the single viewer `c`. */
void ptc(Character *c, const MultiMessage &fmt, ...);

enum {
    OLC_NDX_ROOM,
    OLC_NDX_OBJ,
    OLC_NDX_MOB,
};

/** First free vnum of the given index type in pArea that ch can build in, or -1.
 *  Vnums in skip count as taken (a dry run's planned creates). */
int next_index_in_area( Character *ch, AreaIndexData *pArea, int ndx_type,
                        const std::set<int> *skip = 0 );

/** Resolve the argument of 'create <vnum>|next' for a new room, mob or object:
 *  a free vnum inside an area ch can build in, or -1 with the reason told to ch.
 *  'next' takes the first free vnum of nextArea. Every create path, in game and
 *  over the API, goes through here. */
int olc_create_vnum( Character *ch, AreaIndexData *nextArea, const char *argument, int ndx_type,
                     const std::set<int> *skip = 0 );

/** 'asave changed': write every changed area ch can build in. Returns the
 *  number of areas that failed to save, the saved ones go to 'saved'. */
int save_changed_areas( Character *ch, std::list<AreaIndexData *> &saved );

int next_obj_index( Character *ch, RoomIndexData *r );
int next_room( Character *ch, RoomIndexData *r );
int next_mob_index( Character *ch, RoomIndexData *r );

#endif /* __OLC_H__ */

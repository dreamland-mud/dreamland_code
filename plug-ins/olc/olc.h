// $Id: olc.h,v 1.1.2.14.6.4 2010-09-01 21:20:46 rufina Exp $

// The version info.  Please use this info when reporting bugs.
// It is displayed in the game by typing 'version' while editing.
// Do not remove these from the code - by request of Jason Dinkel
#ifndef __OLC_H__
#define __OLC_H__

#include "onlinecreation.h"
#include "descriptor.h"

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

namespace Scripting {
    class Object;
}

void string_show(Character * ch, char *strch);


#define MAX_MOB 1                /* Default maximum number for resetting mobs */

AreaIndexData *get_area_data(int vnum);

bool show_help(Character * ch, const char *argument);
void show_fenia_triggers(Character *, Scripting::Object *wrapper);
int help_next_free_id();

// Macros

#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

// Prototypes
// mem.c - memory prototypes.
EXIT_DATA *new_exit();
void free_exit(EXIT_DATA * pExit);
EXTRA_DESCR_DATA *new_extra_descr();
void free_extra_descr(EXTRA_DESCR_DATA * pExtra);
OBJ_INDEX_DATA *new_obj_index();
MOB_INDEX_DATA *new_mob_index();

#define stc(t, c) (c)->send_to((t))
#define IS_NPC(c) (c)->is_npc()


const char * get_skill_name( int sn, bool verbose = true );
int olc_handler(Descriptor *d, char *argument);
void ptc(Character *c, const char *fmt, ...);

int next_obj_index( Character *ch, RoomIndexData *r );
int next_room( Character *ch, RoomIndexData *r );
int next_mob_index( Character *ch, RoomIndexData *r );

#endif /* __OLC_H__ */

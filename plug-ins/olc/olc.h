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

bool show_help(Character * ch, const char *argument);
int help_next_free_id();
DLString show_enum_array(const EnumerationArray &array);
DLString show_enum_array_web(const EnumerationArray &array);
void show_behaviors(PCharacter *ch, const GlobalBitvector &behaviors, const Json::Value &props);

#define stc(t, c) (c)->send_to((t))

const char * get_skill_name( int sn, bool verbose = true );
void ptc(Character *c, const char *fmt, ...);

int next_obj_index( Character *ch, RoomIndexData *r );
int next_room( Character *ch, RoomIndexData *r );
int next_mob_index( Character *ch, RoomIndexData *r );

#endif /* __OLC_H__ */

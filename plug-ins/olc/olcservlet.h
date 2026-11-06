#ifndef __OLCSERVLET_H__
#define __OLCSERVLET_H__

class PCharacter;
class Room;
class CaptureBufferHandler;

/** Synthetic security-110 actor for the OLC API servlets (/redit, /oedit).
 *  It is never registered in char_list: OLC subcommands don't move or
 *  re-attach their actor, so a detached char is enough. Output goes to capture. */
PCharacter * olc_servlet_setup_char(Room *room, CaptureBufferHandler *&capture);
void olc_servlet_cleanup_char(PCharacter *ch);

#endif

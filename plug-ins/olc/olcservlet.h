#ifndef __OLCSERVLET_H__
#define __OLCSERVLET_H__

class PCharacter;
class Room;
class CaptureBufferHandler;
class DLString;
struct RoomIndexData;

/** Synthetic security-110 actor for the OLC API servlets (/redit, /oedit).
 *  It is never registered in char_list: OLC subcommands don't move or
 *  re-attach their actor, so a detached char is enough. Output goes to capture. */
PCharacter * olc_servlet_setup_char(Room *room, CaptureBufferHandler *&capture);
void olc_servlet_cleanup_char(PCharacter *ch);

/** Puts text into the actor's editor buffer, so that a following
 *  '<field> paste' / 'ed paste <keyword>' subcommand sets a multi-line field
 *  that would otherwise need the interactive string editor. */
void olc_servlet_seed_buffer(PCharacter *ch, const DLString &text);

/** Non-empty error text when a resolved subcommand must not run over the API:
 *  done/cancel, create, a bare direction (would move the actor), a paste
 *  without text (would paste an empty buffer and wipe the field), or a line
 *  longer than an input line (overflows the handlers' fixed buffers). */
DLString olc_servlet_refusal(const DLString &cmdName, const DLString &arguments, bool hasText);
/** One of the six exit names (north ... down), as redit takes them. */
bool olc_is_direction(const DLString &name);

/** Room prototype fields as XML, the built-in XMLRoom serializer. UTF-8:
 *  XMLDocument::save converts on output. */
DLString olc_room_to_xml(RoomIndexData *pRoom);

#endif

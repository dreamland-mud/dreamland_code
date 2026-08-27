/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MUDTAGS_H
#define MUDTAGS_H

#include <iosfwd>

class Character;
class DLString;

#define TAGS_CONVERT_VIS      (A) // Ask to convert visibility tags.
#define TAGS_CONVERT_COLOR    (B) // Ask to convert color tags.
#define TAGS_ENFORCE_NOCOLOR  (C) // Strip colors, regardless of player settings.
#define TAGS_ENFORCE_WEB      (D) // Convert for web client, regardless of player setttings.
#define TAGS_ENFORCE_NOWEB    (E) // Convert for telnet, regardless of player settings.
#define TAGS_ENFORCE_RAW      (F) // Output non-color tag as they are; don't use ANSI "clear" sequence.

/**
 * Resolve various tags inside a block of text and send result to the 'out' stream.
 */
void mudtags_convert( const char *text, std::ostringstream &out, int flags, Character *ch = NULL );

/**
 * Neutralize player-injected {h web/command tags in free text, in place: the tag
 * (and any command an {hc carries) is dropped, the bare label and colours are kept.
 * Use on any player-authored text that will reach another player's screen -- channels,
 * clan chat, spouse chat. ch is the speaker, a valid viewer (never NULL).
 */
void mudtags_strip_web( DLString &text, Character *ch );

#endif


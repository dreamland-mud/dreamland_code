/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __INFONET_H__
#define __INFONET_H__

class Character;
class Object;
class MultiMessage;

Object * get_pager( Character *ch );
void infonet(const char *string, Character *ch, int min_level);
void infonet( Character *ch, int min_level, const DLString &prefix, const char *fmt, ...);

/* Trilinguality (Trello 2594): per-recipient info channel. `format` is a single
 * MultiMessage covering the whole line (prefix flavour + body), with numbered
 * %N$ codes AND oldact $-codes ($o2 = reader's pager). The format is resolved
 * in each recipient's language and its %N$ args rendered per viewer, then
 * oldact_p resolves the $-codes -- so every listener sees the message in their
 * own display language. Mirrors the existing const char* overload's structure
 * (vfmt then oldact_p), just per recipient instead of once. */
void infonet( Character *ch, int min_level, const MultiMessage &format, ...);

#endif


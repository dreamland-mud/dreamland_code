/* $Id: act.h,v 1.1.2.4 2008/07/26 19:07:04 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __ACT_H__
#define __ACT_H__

#include <stdarg.h>
#include <sstream>
#include <list>
#include <map>

#include "dlstring.h"
#include "multimessage.h"

using namespace std;

class Character;
class NPCharacter;
class Object;
class PCMemoryInterface;

/*
 * TO types for act.
 */
enum {
    TO_NOBODY = -1,
    TO_ROOM,
    TO_NOTVICT,
    TO_VICT,
    TO_CHAR,
    TO_ALL,
    TO_MAX
};

/*--------------------------------------------------------------------------
 * 'act' interface functions 
 *--------------------------------------------------------------------------*/
void oldact( const char *format, Character *ch, 
          const void *arg1, const void *arg2, int type );

void oldact_p( const char *format, Character *ch,
            const void *arg1, const void *arg2, int type, int min_pos );

/** Convert legacy `$`-act codes ($n/$e/$s/...) into the `%`-format the
 *  MsgFormatter consumes. Defined in act.cpp; declared here so the MultiMessage
 *  output path (character.cpp) can run it per resolved language. */
DLString act_to_fmt(const char *s);

/* Trilinguality (Trello 2594, Phase 4): MultiMessage overloads. The format is
 * resolved per recipient in the viewer's language (RU/untranslated -> source
 * literal, byte-identical to the const char* path). oldact sets the act-code
 * flag so act_to_fmt runs per language. */
void oldact( const MultiMessage &format, Character *ch,
          const void *arg1, const void *arg2, int type );
/* Trilinguality (2594): oldact that carries BOTH act-codes AND printf args, so a
 * broadcast/NPC-say line with a %d/%s (e.g. an "in N minutes" timer) resolves per
 * recipient instead of being pre-formatted RU by fmt(0). act-codes occupy fmt args
 * 1-3 ($c->%1, $o->%2, $C->%3 per actChar_to_fmtChar), so number the source's
 * printf placeholders %4$... onward. Varargs are (Character *ch, const void *arg1,
 * const void *arg2, <printf args>) -- the same first three as oldact, then the
 * printf args. Reuses vecho(MM,va_list) (say_fmt's per-recipient path). */
void oldact_fmt( const MultiMessage &format, int type, ... );
void oldact_p( const MultiMessage &format, Character *ch,
            const void *arg1, const void *arg2, int type, int min_pos );

/*--------------------------------------------------------------------------
 * new fmt functions
 *--------------------------------------------------------------------------*/
DLString fmt(Character *to, const char *fmt, ...);
DLString fmt(Character *to, const MultiMessage &fmt, ...);
DLString vfmt(Character *to, const char *format, va_list av);

/*--------------------------------------------------------------------------
 * tell-like output 
 *--------------------------------------------------------------------------*/
void tell_raw( Character *ch, NPCharacter *talker, const char *format, ... );
void say_act( Character *, Character *, const char *, const void *arg = 0 );
void tell_act( Character *, Character *, const char *, const void *arg = 0 );
void tell_dim( Character *, Character *, const char *, const void *arg = 0 );
void tell_fmt( const char *, ... );
void say_fmt( const char *, ... );

/* Trilinguality (Trello 2594): MultiMessage overloads localize the speech frame
 * per recipient. The const char* twins keep the RU frame, so unwrapped callers
 * stay byte-identical -- only a _()-wrapped msg gets a localized frame. */
void say_act( Character *, Character *, const MultiMessage &, const void *arg = 0 );
void tell_act( Character *, Character *, const MultiMessage &, const void *arg = 0 );
void tell_dim( Character *, Character *, const MultiMessage &, const void *arg = 0 );
void tell_fmt( const MultiMessage &, ... );
void tell_raw( Character *ch, NPCharacter *talker, const MultiMessage &format, ... );
/* say_fmt broadcasts to the whole room (many recipients, differing langs) with
 * no single listener anchor, so it composes the frame+content in all three
 * languages into an explicit MultiMessage and lets vecho(MM) resolve per
 * recipient. */
void say_fmt( const MultiMessage &, ... );

/** Display newbie hint message. */
void hint_fmt(Character *ch, const char *format, ...);

/** Output message to mob's master. */
void echo_master(Character *ch, const char *format, ...);

/** Output messages only if target character satisfies a condition. */
void echo_char(Character *ch, bool (Character *), const char *format, ...);
void echo_room(Character *ch, bool (Character *), const char *format, ...);
void echo_notvict(Character *ch, Character *victim, bool (Character *), const char *format, ...);

/* Trilinguality (Trello 2594): MultiMessage overloads of the above, so `_()`
 * wrapped calls resolve per recipient. Additive -- MultiMessage never converts
 * to/from const char*, so const char* callers keep their overload. */
void hint_fmt(Character *ch, const MultiMessage &format, ...);
void echo_master(Character *ch, const MultiMessage &format, ...);
void echo_char(Character *ch, bool (Character *), const MultiMessage &format, ...);
void echo_room(Character *ch, bool (Character *), const MultiMessage &format, ...);
void echo_notvict(Character *ch, Character *victim, bool (Character *), const MultiMessage &format, ...);


/** Output given list of values in N columns of particular width. */
DLString print_columns(const list<DLString> &names, int width, int columns);

#endif

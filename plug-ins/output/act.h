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

/*--------------------------------------------------------------------------
 * new fmt functions 
 *--------------------------------------------------------------------------*/
DLString fmt(Character *to, const char *fmt, ...);
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

/** Display newbie hint message. */
void hint_fmt(Character *ch, const char *format, ...);

/** Output message to mob's master. */
void echo_master(Character *ch, const char *format, ...);

/** Output messages only if target character satisfies a condition. */
void echo_char(Character *ch, bool (Character *), const char *format, ...);
void echo_room(Character *ch, bool (Character *), const char *format, ...);
void echo_notvict(Character *ch, Character *victim, bool (Character *), const char *format, ...);

#endif

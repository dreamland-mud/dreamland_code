/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __INTERP_H__
#define __INTERP_H__

class Character;

bool        interpret        ( Character *ch, const char *line );
void        interpret_fmt        ( Character *ch, const char *format, ... );
void        interpret_raw        ( Character *ch, const char *cmd, const char *format = "", ... );
bool    interpret_cmd        ( Character *ch, const char *cmd, const char *argsFormat, ... );

/*
 * The character whose command is running right now, or 0 between commands.
 * Lets the wait/daze setters tell a char's own command lag from lag that
 * someone else puts on it.
 */
Character * interpret_actor( );

/*
 * Grows by one with every command interpret() runs, so a check made twice
 * inside one command (preRun and run) can tell itself from a repeated one.
 */
long interpret_serial( );

class InterpretActorGuard {
public:
    InterpretActorGuard( Character *ch );
    ~InterpretActorGuard( );
private:
    Character *saved;
};


void do_say( Character *, const char * );
void do_yell( Character *, const char * );

#endif

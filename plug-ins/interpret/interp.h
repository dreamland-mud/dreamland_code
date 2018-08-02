/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __INTERP_H__
#define __INTERP_H__

class Character;

bool	interpret	( Character *ch, const char *line );
void	interpret_fmt	( Character *ch, const char *format, ... );
void	interpret_raw	( Character *ch, const char *cmd, const char *format = "", ... );
bool    interpret_cmd	( Character *ch, const char *cmd, const char *argsFormat, ... );

#endif

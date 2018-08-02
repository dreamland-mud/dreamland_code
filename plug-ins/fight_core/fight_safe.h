/* $Id$
 *
 * ruffina, 2004
 */
#ifndef FIGHT_SAFE_H
#define FIGHT_SAFE_H

class Character;

bool 	is_safe( Character *ch, Character *victim );
bool 	is_safe_nomessage( Character *ch, Character *victim );
bool	is_safe_rspell( Character *victim );
bool	is_safe_rspell	( short level, Character *victim );
bool	is_safe_rspell_nom( short level, Character *victim );

#endif

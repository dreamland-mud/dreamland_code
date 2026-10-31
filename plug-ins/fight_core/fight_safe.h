/* $Id$
 *
 * ruffina, 2004
 */
#ifndef FIGHT_SAFE_H
#define FIGHT_SAFE_H

class Character;
class Affect;

bool         is_safe( Character *ch, Character *victim );
bool         is_safe_nomessage( Character *ch, Character *victim, bool verbose=false );
bool         groupmate_attack_held( Character *ch, Character *victim );
bool         is_safe_attack( Character *ch, Character *victim );
bool        is_safe_rspell( short level, Character *victim, bool verbose = true );
bool        is_safe_rspell( Affect *paf, Character *victim, bool verbose = true );

#endif

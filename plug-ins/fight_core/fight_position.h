/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __FIGHT_POSITION_H__
#define __FIGHT_POSITION_H__

class Character;

void stop_fighting( Character *ch, bool fBoth );
void set_fighting( Character *ch, Character *victim );
void update_pos( Character *victim );

void        set_violent( Character *ch, Character *victim, bool fAlways );
void        set_thief( Character *ch );
void        set_ghost( Character *ch );
void        set_slain( Character *ch );
void        set_killer( Character *ch );
void        set_violent( Character *ch );

#define SET_DEATH_TIME(ch)        if (!(ch)->is_npc( )) { (ch)->getPC( )->last_death_time = MAX_DEATH_TIME; }
#define UNSET_DEATH_TIME(ch)        if (!(ch)->is_npc( )) { (ch)->getPC( )->last_death_time = -1; }

#endif

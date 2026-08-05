/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __FIGHT_POSITION_H__
#define __FIGHT_POSITION_H__

class Character;

void stop_fighting( Character *ch, bool fBoth );

/** Start a fight. 'standUp' is what puts ch on their feet and shakes off sleep;
 *  pass false for a character who is being attacked while down, so that the whole
 *  round lands at the position penalty before they get up.
 */
void set_fighting( Character *ch, Character *victim, bool standUp = true );

/** On your feet and fighting: also releases the furniture you were resting on. */
void stand_up_to_fight( Character *ch );

/** Called once per melee round on the character being attacked: someone caught
 *  lying, sitting or asleep eats that whole round at the position penalty and
 *  only then wakes up, stands and frees the furniture.
 */
void stand_up_after_round( Character *victim );

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

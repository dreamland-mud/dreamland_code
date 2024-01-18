/* $Id: fight.h,v 1.1.2.10 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef FIGHT_H
#define FIGHT_H

#include <stdio.h>

#include "bitstring.h"
#include "fight_position.h"
#include "fight_safe.h"
#include "weapons.h"
#include "attacks.h"
#include "damageflags.h"

class Character;
class PCharacter;
class NPCharacter;
class Object;
class Skill;
class Affect;
class Profession;

/* fight.cpp */
void        violence_update( );
void        multi_hit( Character *ch, Character *victim, string command = "" );
void        multi_hit_nocatch( Character *ch, Character *victim, string command = "" );
bool        next_attack( Character *ch, Character *victim, Skill &skill, int coef );
void        one_hit( Character *ch, Character* victim, bool secondary = false, string command = "" );
void        one_hit_nocatch( Character *ch, Character* victim, bool secondary = false, string command = "" );
bool        damage( Character *ch, Character *victim, int dam, int sn, int dam_type, bool show, bitstring_t dam_flag);
bool        damage_nocatch( Character *ch, Character *victim, int dam, int sn, int dam_type, bool show, bitstring_t dam_flag);
bool        damage( Affect *paf, Character *victim, int dam, int sn, int dam_type, bool show, bitstring_t dam_flag);
bool        damage_nocatch( Affect *paf, Character *victim, int dam, int sn, int dam_type, bool show, bitstring_t dam_flag);
void        rawdamage( Character *ch, Character *victim, int dam_type, int dam, bool show, const DLString &deathReason );
void        rawdamage_nocatch( Character *ch, Character *victim, int dam_type, int dam, bool show, const DLString &deathReason );
void        damage_to_obj(Character *ch,Object *wield, Object *worn, int damage);
int        move_dec( Character *ch );
void damapply_class(Character *ch, int &dam);
int second_weapon_chance(Profession *prof, Object *weapon);


void yell_panic( Character *ch, Character *victim, const char *msgBlind , const char *msg, const char *label = 0 );


/* gaining experience (fight_exp.cpp) */
void        group_gain( Character *ch, Character *victim, Character *realKiller = 0 );

/* death handling routines (fight_death.cpp) */
void        raw_kill( Character* victim, bitstring_t flags, Character* ch, const DLString &label, int damtype );
Object * bodypart_create( int vnum, Character *ch, Object *corpse );

/* subroutines (fight_subr.cpp) */
void        check_assist(Character *ch,Character *victim);
bool        check_stun( Character *ch, Character *victim ); 
bool        check_bare_hands( Character *ch );
void        check_bloodthirst( Character *ch );


#define IS_QUICK(ch) ((((ch)->is_npc() && IS_SET((ch)->getNPC()->off_flags,OFF_FAST)) \
                      || IS_AFFECTED((ch),AFF_HASTE)) && !IS_AFFECTED((ch),AFF_SLOW))

#define IS_SLOW(ch) (IS_AFFECTED((ch),AFF_SLOW) && !(((ch)->is_npc() && IS_SET((ch)->getNPC()->off_flags,OFF_FAST)) \
                      || IS_AFFECTED((ch),AFF_HASTE)))

#endif

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

/* fight.cpp */
void        violence_update( );
void        multi_hit( Character *ch, Character *victim );
void        multi_hit_nocatch( Character *ch, Character *victim );
bool        next_attack( Character *ch, Character *victim, Skill &skill, int coef );
void        one_hit( Character *ch, Character* victim, bool secondary = false);
void        one_hit_nocatch( Character *ch, Character* victim, bool secondary = false);
bool        damage( Character *ch, Character *victim, int dam, int sn, int dam_type, bool show, bitstring_t dam_flag = 0 );
bool        damage_nocatch( Character *ch, Character *victim, int dam, int sn, int dam_type, bool show, bitstring_t dam_flag = 0 );
void        rawdamage( Character *ch, Character *victim, int dam_type, int dam, bool show );
void        rawdamage_nocatch( Character *ch, Character *victim, int dam_type, int dam, bool show );
void        damage_to_obj(Character *ch,Object *wield, Object *worn, int damage);
int        move_dec( Character *ch );


#define FYP_SLEEP    (A)
#define FYP_VICT_ANY (B)
void yell_panic( Character *ch, Character *victim, const char *msgBlind = NULL, const char *msg = NULL, int flags = 0 );


/* gaining experience (fight_exp.cpp) */
void        group_gain( Character *ch, Character *victim );

/* death handling routines (fight_death.cpp) */
#define    FKILL_CRY          (A)
#define    FKILL_GHOST        (B)
#define    FKILL_CORPSE       (C)
#define    FKILL_PURGE        (D)
#define    FKILL_MOB_EXTRACT  (E)
#define    FKILL_REABILITATE  (F)
void        raw_kill( Character* victim, int part, Character* ch, int flags );
void        death_cry( Character *ch, int part = -1 );
void        pk_gain( Character *ch, Character *victim );

/* subroutines (fight_subr.cpp) */
#define FOREST_ATTACK  1
#define FOREST_DEFENCE 2
void        check_assist(Character *ch,Character *victim);
bool        check_stun( Character *ch, Character *victim ); 
bool        check_bare_hands( Character *ch );
void        check_bloodthirst( Character *ch );


#define IS_QUICK(ch) ((((ch)->is_npc() && IS_SET((ch)->getNPC()->off_flags,OFF_FAST)) \
                      || IS_AFFECTED((ch),AFF_HASTE)) && !IS_AFFECTED((ch),AFF_SLOW))

#define IS_SLOW(ch) (IS_AFFECTED((ch),AFF_SLOW) && !(((ch)->is_npc() && IS_SET((ch)->getNPC()->off_flags,OFF_FAST)) \
                      || IS_AFFECTED((ch),AFF_HASTE)))

#endif

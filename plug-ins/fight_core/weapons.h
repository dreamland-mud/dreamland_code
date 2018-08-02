/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEAPONS_H
#define WEAPONS_H

class Character;
class Object;
class Skill;

Object * get_wield( Character *ch, bool secondary );
int	 get_weapon_sn( Character *ch, bool secondary );
int 	 get_weapon_sn( Object *wield );
Skill *  get_weapon_skill( Object *wield );

#endif

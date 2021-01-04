/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEAPONS_H
#define WEAPONS_H

class Character;
class Object;
class Skill;
struct obj_index_data;

Object * get_wield( Character *ch, bool secondary );
int         get_weapon_sn( Character *ch, bool secondary );
int          get_weapon_sn( Object *wield );
Skill *  get_weapon_skill( Object *wield );

/** Return average damage for a weapon. */
int weapon_ave(Object *wield);

/** Return average damage for a weapon prototype. */
int weapon_ave(struct obj_index_data *pWield);


#endif

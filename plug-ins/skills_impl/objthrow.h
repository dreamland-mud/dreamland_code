
/* $Id: objthrow.h,v 1.1.2.1 2007/05/02 03:05:14 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __OBJTHROW_H__
#define __OBJTHROW_H__

class Character;
class Object;

bool check_obj_dodge( Character *ch, Character *victim, Object *obj, int bonus );
int send_arrow( Character *ch, Character *victim, Object *arrow, int door, int chance ,int bonus);

#endif

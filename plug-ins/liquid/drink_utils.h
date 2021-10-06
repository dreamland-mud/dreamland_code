/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DRINK_UTILS_H__
#define __DRINK_UTILS_H__

class Object;
class Character;

bool drink_is_closed( Object *obj, Character *ch );
void pour_out( Object * out );
#endif

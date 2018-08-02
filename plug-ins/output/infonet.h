/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __INFONET_H__
#define __INFONET_H__

class Character;
class Object;

Object * get_pager( Character *ch );
void infonet(const char *string, Character *ch, int min_level);

#endif


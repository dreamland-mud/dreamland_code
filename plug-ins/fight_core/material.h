/* $Id: material.h,v 1.1.2.3 2009/01/16 13:50:47 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef MATERIAL_H
#define MATERIAL_H

#include "def.h"

class Object;
class Character;

bool material_is_typed( Object *, int );
bool material_is_flagged( Object *, int );
int material_immune( Object *, Character * );
int material_burns( Object * );

enum {
    SWIM_UNDEF = 0,
    SWIM_NEVER,
    SWIM_ALWAYS,
};
int material_swims( Object * );

#endif

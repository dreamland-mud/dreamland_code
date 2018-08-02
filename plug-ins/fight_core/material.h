/* $Id: material.h,v 1.1.2.3 2009/01/16 13:50:47 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef MATERIAL_H
#define MATERIAL_H

#include "def.h"

class Object;
class Character;

// material types
#define MAT_NONE        (0) 
#define MAT_ABSTRACT    (A)
#define MAT_METAL       (B)
#define MAT_GEM         (E)
#define MAT_ELEMENT     (F)
#define MAT_MINERAL     (G)
#define MAT_ORGANIC     (H)
#define MAT_WOOD        (I)
#define MAT_CLOTH       (J)
                       
                       
// material flags      
#define MAT_MELTING     (A)
#define MAT_FRAGILE     (B)
#define MAT_INDESTR     (C)
#define MAT_TOUGH       (D)

struct material_t {
    const char *name;
    int burns;
    int floats;
    int type;
    int flags;
    int vuln;
    const char *rname;
};

extern const struct material_t material_table [];

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

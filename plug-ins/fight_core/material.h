/* $Id: material.h,v 1.1.2.3 2009/01/16 13:50:47 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef MATERIAL_H
#define MATERIAL_H

#include "bitstring.h"

class Object;
class Character;

bool material_is_typed( Object *, int );
bool material_is_typed( const char *, int );

/** Material types this character can never wear or wield, e.g. metal for druids. */
int material_types_forbidden( Character * );

/** Would iron or steel among these materials pin this char down? A mortal faery
 *  wearing or wielding cold iron can't blink, unless imm/res (the char's imm_flags /
 *  res_flags to test against) hold iron. Mirrors the Fenia predicate
 *  .tmp.object.wearsColdIron used by skillcommand/blink/run. */
bool material_cold_iron_pins( Character *ch, const char *materials, bitstring_t imm, bitstring_t res );

bool material_is_flagged( Object *, int );
bool material_is_flagged( const char *, int );
int material_immune( Object *, Character * );
int material_burns( Object * );
int material_burns( const char * );
DLString material_rname(Object *obj);
DLString material_rname(const char *materials);
DLString material_uaname(Object *obj);
DLString material_uaname(const char *materials);

enum {
    SWIM_UNDEF = 0,
    SWIM_NEVER,
    SWIM_ALWAYS,
};
int material_swims( Object * );
int material_swims( const char *materials );
int material_rho( Object * );
int material_rho( const char * );
int material_hardness( Object * );
int material_hardness( const char * );

#endif

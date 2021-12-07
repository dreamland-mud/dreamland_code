/* $Id: material.h,v 1.1.2.3 2009/01/16 13:50:47 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef MATERIAL_H
#define MATERIAL_H

class Object;
class Character;

bool material_is_typed( Object *, int );
bool material_is_typed( const char *, int );
bool material_is_flagged( Object *, int );
bool material_is_flagged( const char *, int );
int material_immune( Object *, Character * );
int material_burns( Object * );
int material_burns( const char * );
DLString material_rname(Object *obj);
DLString material_rname(const char *materials);

enum {
    SWIM_UNDEF = 0,
    SWIM_NEVER,
    SWIM_ALWAYS,
};
int material_swims( Object * );
int material_swims( const char *materials );

#endif

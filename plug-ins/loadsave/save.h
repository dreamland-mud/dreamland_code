/* $Id: save.h,v 1.1.2.1.6.2 2007/09/11 00:34:16 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#ifndef __SAVE_H__
#define __SAVE_H__

#include <stdio.h>

class DLString;
class Character;
class NPCharacter;
class PCharacter;
class Object;
class Room;

#define MAX_NEST	100
extern Object *	rgObjNest [MAX_NEST];

void fwrite_mob( NPCharacter *mob, FILE *fp );
void fwrite_char( PCharacter *ch,  FILE *fp );
void fwrite_obj( Character *ch,  Object  *obj, FILE *fp, int iNest );
void fwrite_obj_0( Character *ch,  Object  *obj, FILE *fp, int iNest );
void fwrite_pet( NPCharacter *pet, FILE *fp);

NPCharacter * fread_mob( FILE *fp );
void fread_char( PCharacter *ch,  FILE *fp );
void fread_pet( PCharacter *ch,  FILE *fp );
void fread_mlt( PCharacter *ch, FILE *fp );
void fread_obj( Character *ch,  Room *room, FILE *fp );

// Saving and Load drops

extern bool create_obj_dropped;

void load_drops( );
void load_single_objects_folder( char * subdir, bool remove_after );
void load_room_objects( Room *room, char * path, bool remove_after );

void load_dropped_mobs( );
void load_single_mobiles_folder( char * subdir, bool remove_after );
void load_room_mobiles( Room *room, char * path, bool remove_after );

void save_items( Room *room );
void save_items_at_holder( Object * obj );
void save_room_objects( Room *room );

void save_mobs( Room *room );
void save_mobs_at( Character *ch );
void save_room_mobiles( Room *room );

/*
 * save charmed creatures
 */
void save_creature( NPCharacter *ch );
void unsave_creature( NPCharacter *ch );

#endif

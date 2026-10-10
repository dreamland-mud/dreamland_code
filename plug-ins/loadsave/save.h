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

#define MAX_NEST        100
extern Object *        rgObjNest [MAX_NEST];

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
/** Mob reform: one boot-log line about pre-reform saved body lines. */
void saved_mobiles_report( );
void load_single_mobiles_folder( char * subdir, bool remove_after );
void load_room_mobiles( Room *room, char * path, bool remove_after );

void save_items( Room *room );
void save_items_at_holder( Object * obj );
void save_room_objects( Room *room );

/*
 * One-shot economy migration of saved objects. fight_core (itemeconomy.cpp)
 * sets both from fight/item_weight.json "migration". obj_econ_rev is the
 * current model revision (0 = off); every new object is stamped with it and
 * fwrite_obj saves it as EconRev. fread_obj calls the hook for an object saved
 * with an older revision, twice: stage 0 before the object is placed (weight,
 * so carry weight stays balanced), stage 1 once its Fenia wrapper is linked
 * (cost). The hook answers true when it applied the change; the object then
 * takes the current revision. In dry-run it only logs and answers false.
 */
typedef bool (*ObjEconMigrateFn)( Object *obj, int savedRev, int stage );
extern int obj_econ_rev;
extern ObjEconMigrateFn obj_econ_migrate_fn;

void save_mobs( Room *room );
void save_mobs_at( Character *ch );
void save_room_mobiles( Room *room );

/*
 * save charmed creatures
 */
void save_creature( NPCharacter *ch );
void unsave_creature( NPCharacter *ch );

#endif

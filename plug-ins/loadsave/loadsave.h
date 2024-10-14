/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __LOADSAVE_H__
#define __LOADSAVE_H__

#include <list>
#include "grammar_entities.h"
#include "grammar_entities_impl.h"
#include "dlstring.h"
#include "arg_utils.h"
#include "affectflags.h"
#include "itemflags.h"
#include "compatflags.h"
#include "char_weight.h"
#include "char_body.h"
#include "money_utils.h"
#include "itemutils.h"

using namespace std;

class Affect;
class Object;
class Room;
class Character;
class PCharacter;
class NPCharacter;
class FlagTable;

struct mob_index_data;
struct obj_index_data;

#define FCREATE_NOCOUNT   (A)
#define FCREATE_NOAFFECTS (B)

NPCharacter *        create_mobile        ( mob_index_data *pMobIndex );
NPCharacter *        create_mobile_nocount( mob_index_data * );
NPCharacter *        create_mobile_org( mob_index_data *pMobIndex, int flags );
void create_mob_affects(NPCharacter *mob);
void                clone_mobile        ( NPCharacter *parent, NPCharacter *clone);
Object *        create_object        ( obj_index_data *pObjIndex, short level );
Object *        create_object_nocount ( obj_index_data *pObjIndex, short level );
Object *        create_object_org (obj_index_data *pObjIndex,short level,bool Count);
void                clone_object         ( Object *parent, Object *clone );

void        affect_to_obj    ( Object *, const Affect *paf );
void        affect_remove_obj( ::Object *, Affect *paf, bool verbose = false );
void        affect_enhance   ( Object *obj, const Affect * );
void        affect_modify    ( Character *ch, Affect *paf, bool fAdd );
void        affect_check         ( Character *ch, Affect *old_paf);
void        affect_to_char         ( Character *ch, Affect *paf );
void        affect_remove         ( Character *ch, Affect *paf, bool verbose = false );
void        affect_strip         ( Character *ch, int sn, bool verbose = false  );
void        affect_strip         ( Object *obj, int sn, bool verbose = false );
void        affect_bit_strip ( Character *ch, const FlagTable *table, int bits, bool verbose = false );
void        affect_join         ( Character *ch, Affect *paf );
void        postaffect_to_char( Character *ch, int sn, int duration );

void        char_from_room        ( Character *ch );
void        char_to_room        ( Character *ch, Room *pRoomIndex );
void        obj_to_char        ( Object *obj, Character *ch );
void        obj_from_char        ( Object *obj );
void        obj_from_room        ( Object *obj );
void        obj_to_room        ( Object *obj, Room *pRoomIndex );
void        obj_to_obj        ( Object *obj, Object *obj_to );
void        obj_to_obj_random( Object *item, Object *obj_to );
void        obj_from_obj        ( Object *obj );
void obj_dump_content(Object *obj);

void undig_earthquake( Character *ch );
void undig( Character *ch );
void strip_camouflage( Character *ch );
void check_camouflage( Character *ch, Room *to_room );
void strip_hide_and_fade(Character *ch);
void strip_invisibility(Character *ch);
void strip_improved_invisibility(Character *ch);

void        extract_obj        ( Object *obj, const char *message = 0 );
void        extract_obj_nocount        ( Object *obj );
void        extract_obj_1        ( Object *obj, bool count, const char *message = 0 );
void        char_from_list( Character *ch, Character **list );
void        char_to_list( Character *ch, Character **list );
void        obj_to_list( Object * );
void        obj_from_list( Object * );
void        extract_mob_baddrop( NPCharacter * );
void        extract_mob_dropped( NPCharacter * );
bool        mprog_extract( Character *ch, bool count );

int                count_obj_list        ( obj_index_data *obj, Object *list );
Character *        get_char_room        ( Character *ch, const DLString &arg, int flags = 0 );
Character *        get_char_room        ( Character *ch, Room *room, const DLString &arg, int flags = 0 );
Character *        get_char_room        ( Character *ch, Room *room, const DLString &arg, int *number, int flags = 0 );
Character *        get_char_world        ( Character *ch, const char *argument, int flags = 0 );
Character *        get_char_world        ( Character *ch, const DLString &arg, int flags = 0 );
PCharacter *        get_player_world( Character *ch, const char *arg, bool fSeenOnly = true );
Object *        get_obj_list        ( Character *ch, const DLString &cArg, Object *list, DLString pocket = "" );
Object *        get_obj_carry        ( Character *ch, const DLString & );
Object *        get_obj_carry        ( Character *ch, char *argument );
Object *        get_obj_wear        ( Character *ch, const char *argument );
Object *        get_obj_wear        ( Character *ch, const DLString & );
Object *        get_obj_here        ( Character *ch, const DLString & );
Object *        get_obj_here        ( Character *ch, char *argument );
Object *        get_obj_world        ( Character *ch, char *argument );
DLString        get_obj_name_list( Object *target, Object *list, Character *ch );
Object *        get_obj_world_unique( int vnum, Character *ch );
list<Object *>        get_objlist_world_unique( int vnum, Character *ch );
Object *        get_obj_room_unique( Room *room, int itype, Character *ch );
DLString         get_char_name_list( Character *target, Character *list, Character *ch );
Object *        get_obj_room( Character *ch, const char *argument );
Object *        get_obj_room( Character *ch, const DLString & );
Object *        get_obj_wear_victim( Character *victim, const DLString &cArg, Character *ch );
Object *        get_obj_room_vnum( Room *room, int vnum );
Object *        get_obj_carry_vnum( Character *ch, int vnum );
Object *        get_obj_carry_type( Character *ch, int type );
Object *        get_obj_room_type( Character *ch, int type );
Object *        get_obj_room_type( Room *room, int type );
Object *        get_obj_list_type( Character *ch, int type, Object *list );
Object *        get_obj_list_type( Character *ch, const DLString &cArg, int type, Object *list );
Object * get_obj_list_vnum( Character *ch, int vnum, Object *list );
int                count_obj_in_obj( Object *container );
int                count_obj_in_obj( Object *container, int itype );
Object *        get_obj_wear_carry( Character *ch, const DLString &cArgument, Character *looker = 0 );
Object * get_key_carry( Character *ch, int vnum );

bool can_see_god(Character *ch, Character *god);
bool obj_has_name( Object *obj, const DLString &arg, Character *ch );
long long get_arg_id( const DLString &cArgument );
bool obj_has_name_or_id( Object *obj, const DLString &arg, Character *ch, long long id );
Object *find_pit_for_obj(Object *obj);
Object *find_pit_in_room(int roomVnum);

bool eyes_blinded( Character *ch );
bool eyes_darkened( Character *ch );
void eyes_blinded_msg( Character *ch );

#define IS_WEAPON_STAT(obj,stat)(IS_SET((obj)->value4(),(stat)))

#define IS_PIT(obj)        (obj->item_type == ITEM_CONTAINER \
                            && IS_SET(obj->value1(), CONT_PIT))

#define IS_CHARMED(ch)  (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)

#define IS_MOB_VAMPIRE(ch) ((ch)->is_npc( ) && IS_SET((ch)->act, ACT_VAMPIRE))

#endif

/* $Id: handler.h,v 1.1.2.23.6.13 2010-09-01 21:20:43 rufina Exp $
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
#ifndef HANDLER_H
#define HANDLER_H

#include "loadsave.h"
#include "wearloc_utils.h"
#include "fight_safe.h"
#include "fight_position.h"
#include "follow_utils.h"
#include "grammar_entities.h"

class Character;
class Object;
class NPCharacter;
struct mob_index_data;

int	    count_users(Object *obj);
void        get_money_here( Object *list, int &gold, int &silver );
Object *    create_money( int gold, int silver );
DLString    describe_money( int gold, int silver, const Grammar::Case &gcase );
Character * find_char( Character *ch, const char *argument, int door, int *range, bool message = true );

void        write_bug_file( Character *ch, const DLString &filename, const char *txt );

bool can_drop_obj( Character *ch, Object *obj, bool verbose = false );
void do_get_raw( Character *ch, Object *obj );
bool do_get_raw( Character *ch, Object *obj, Object *container );
void do_get_all_raw( Character *ch, Object *container );
bool oprog_get( Object *obj, Character *ch );
void reboot_anatolia( void);
void eyes_blinded_msg( Character *ch );

void do_look_auto( Character *ch, Room *room, bool fBrief = false, bool fShowMount = true );
bool oprog_examine( Object *obj, Character *ch, const DLString &arg = DLString::emptyString );


void do_quit( Character *, const char * );
void do_say( Character *, const char * );
void do_yell( Character *, const char * );

#define    FEXTRACT_TOTAL  (A)
#define    FEXTRACT_COUNT  (B)
#define    FEXTRACT_LASTFOUGHT  (C)
void	extract_dead_player( PCharacter *, int flags );
void	extract_char( Character *, bool fCount = true );

#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->room_flags,		    \
				    ROOM_INDOORS))

#define IS_TRUSTED(ch,level)	(( ch->get_trust() ) >= (level))

#define	SHADOW_ACTIVE		50	
#define SHADOW(ch)	    (HAS_SHADOW(ch) && number_percent() > SHADOW_ACTIVE)
#define HALF_SHADOW(ch)	    (HAS_SHADOW(ch) && number_percent() > SHADOW_ACTIVE/2)
#define HAS_SHADOW(ch)	    (!(ch)->is_npc() && (ch)->getPC()->shadow >= 0)


#endif

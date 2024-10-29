/* $Id$
 *
 * ruffina, 2004
 */
#include <list>
#include <string.h>
#include "loadsave.h"
#include "save.h"
#include "string_utils.h"
#include "grammar_entities_impl.h"
#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "clan.h"
#include "skillreference.h"
#include "behavior.h"
#include "descriptor.h"
#include "act.h"
#include "merc.h"
#include "def.h"

WEARLOC(none);
CLAN(ruler);
GSN(manacles);

// Parse string argument as if it contains entity ID.
long long get_arg_id( const DLString &cArgument )
{
    DLString arg( cArgument );
    arg = arg.getOneArgument( );
    LongLong id;

    if (arg.size( ) < 10)
        return 0;
    if (!is_number( arg.c_str( ) ))
        return 0;

    try {
        id.fromString( arg );
    } catch (ExceptionBadType &e) {
        return 0;
    }

    return id;
}    

// Return true if arg matches one of the character names or short descriptions.
// Doppel is already applied.
static bool char_has_name(Character *target, const DLString &arg)
{
    // TODO getNameP doesn't return all languages. Make PC/NPC specific? 
    return is_name(arg.c_str(), target->getNameP('7').c_str());
}

/** Return true if ch can give orders to the victim. */
static bool char_can_order( Character *ch, Character *victim )
{
    if (ch == victim)
        return false;

    if (ch->in_room != victim->in_room)
        return false;

    if (ch->getClan( ) == clan_ruler && victim->isAffected(gsn_manacles))
        return true;
        
    if (!IS_CHARMED(victim))
        return false;
    
    if (victim->master != ch)
        return false;
        
    if (victim->is_immortal( ) 
        && victim->get_trust( ) >= ch->getModifyLevel( ))
        return false;
    
    return true;
}



/*
 * Find a char in the room.
 */
Character *get_char_room( Character *ch, const DLString & constArgument, int flags )
{
    return get_char_room( ch, ch->in_room, constArgument, flags );
}

Character * get_char_room( Character *ch, Room *room, const DLString &constArgument, int flags )
{
    char arg[MAX_INPUT_LENGTH], argument[MAX_INPUT_LENGTH];
    int number;

    strcpy( argument, constArgument.c_str( ) );
    number = number_argument( argument, arg );
    return get_char_room( ch, room, arg, &number, flags );
}

/*
 * Find a char in the room.
 */
Character *get_char_room( Character *ch, Room *room, const DLString &constArgs, int *number, int flags )
{
    Character *rch;
    int count;
    int ugly;
    DLString argument = constArgs;
    long long id = get_arg_id( argument );

    if (room == 0)
        return 0;

    count  = 0;
    ugly   = 0;

    if (arg_is_self( argument ))
        return ch;

    if (arg_is_ugly( argument ))
        ugly = 1;

    for (rch = room->people; rch != 0; rch = rch->next_in_room)
    {
        if (!ch->can_sense( rch ))
            continue;

        if (!IS_SET(flags, FFIND_INVISIBLE) && !ch->can_see( rch ))
            continue;

        if (IS_SET(flags, FFIND_FOR_ORDER) && !char_can_order(ch, rch))
            continue;

        if (IS_SET(flags, FFIND_FOLLOWER) && rch->leader != ch)
            continue;

        if (IS_SET(flags, FFIND_MOB_ONLY) && !rch->is_npc())
            continue;

        if (IS_SET(flags, FFIND_PLR_ONLY) && rch->is_npc())
            continue;

        if (ugly) {
            // All vampires react to vampiric keywords by default.
            if (IS_VAMPIRE(rch) || IS_MOB_VAMPIRE(rch))
                if (++count == *number)
                    return rch;
        }

        Character *tch;
        
        tch = rch->getDoppel( ch ); // FIXME doppel logic here is different from get_char_world

        if (id && tch->getID( ) == id)
            return rch;
        if (id || !char_has_name(tch, argument))
            continue;

        if (++count == *number)
            return rch;
    }

    *number -= count;

    return 0;
}



/*
 * Find a char in the world.
 */
Character *get_char_world( Character *ch, const DLString &arg, int flags )
{
    return get_char_world( ch, arg.c_str( ), flags );
}
Character *get_char_world( Character *ch, const char *cArgument, int flags )
{
        char arg[MAX_INPUT_LENGTH];
        char arg_buf[strlen(cArgument)+1];
        char *argument = arg_buf;
        Character *wch;
        int number;
        int count;
       
        strcpy( arg_buf, cArgument );
        long long id = get_arg_id( argument );
        number = number_argument( argument, arg );

        if ( ( wch = get_char_room( ch, ch->in_room, arg, &number, flags ) ) != 0 )
                return wch;

        count  = 0;
        for ( wch = char_list; wch != 0 ; wch = wch->next )
        {
                Character *dch = IS_SET(flags, FFIND_DOPPEL) ? wch->getDoppel( ch ) : wch;

                if (dch->in_room == 0)
                    continue;
                if (dch->in_room == ch->in_room)
                    continue;
                if (!IS_SET(flags, FFIND_INVISIBLE) && !ch->can_see( dch ))
                    continue;
                if (IS_SET(flags, FFIND_FOLLOWER) && dch->leader != ch)
                    continue;
                if (IS_SET(flags, FFIND_SAME_AREA) && ch->in_room->area != dch->in_room->area)
                    continue;
                if (id && dch->getID( ) != id)
                     continue;
                if (!id && !char_has_name(dch, arg))
                    continue;
                if (IS_SET(flags, FFIND_MOB_ONLY) && !dch->is_npc())
                    continue;
                if (IS_SET(flags, FFIND_PLR_ONLY) && dch->is_npc())
                    continue;

                if (++count >= number)
                    return wch;
        }

        return 0;
}

/*
 * Find an obj in a list.
 *
 * Change get_obj_here if correct this function
 *
 */
Object *get_obj_list( Character *ch, const DLString &cArg, Object *list, DLString pocket )
{
        char arg[MAX_INPUT_LENGTH];
        char argument[MAX_INPUT_LENGTH];
        Object *obj;
        int number;
        int count;
        long long id;
       
        strcpy(argument, cArg.c_str( ));
        id = get_arg_id( argument );
        number = number_argument( argument, arg );
        count  = 0;
        for ( obj = list; obj != 0; obj = obj->next_content )
        {
            if (!ch->can_see( obj ) && !ch->can_hear( obj ))
                continue;
           
            if (id && obj->getID( ) != id)
                continue;

            if (!id && !obj_has_name( obj, arg, ch ))
                continue;

            if (!pocket.empty( ) && obj->pocket != pocket)
                continue;
            
            if (pocket.empty( ) && !obj->pocket.empty( ))
                continue;

            if (id || ++count == number)
                return obj;
        }

        return 0;
}

Object * get_obj_carry( Character *ch, const DLString & constArgument )
{
    char argument[MAX_INPUT_LENGTH];
    strcpy(argument, constArgument.c_str( ));
    return get_obj_carry( ch, argument );
}

/*
 * Find an obj in player's inventory.
 *
 * Change get_obj_here if correct this function
 *
 */
Object *get_obj_carry( Character *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        Object *obj;
        int number;
        int count;
        long long id = get_arg_id( argument );

        number = number_argument( argument, arg );
        count  = 0;

        for ( obj = ch->carrying; obj != 0; obj = obj->next_content )
        {
                if ( obj->wear_loc == wear_none
                        && (ch->can_see( obj ) || ch->can_hear( obj ))
                        && ((id && obj->getID( ) == id) 
                             || (!id && obj_has_name( obj, arg, ch ))))
                {
                        if (id || ++count == number )
                                return obj;
                }
        }

        return 0;
}

/*
 * Find an obj in player's equipment.
 *
 * Change get_obj_here if correct this function
 *
 */
Object *get_obj_wear( Character *ch, const DLString &arg )
{
    return get_obj_wear( ch, arg.c_str( ) );
}
Object *get_obj_wear( Character *ch, const char *cargument )
{
    char arg[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    Object *obj;
    int number;
    int count;
    long long id = get_arg_id( cargument );
    
    strcpy(argument, cargument);
    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = ch->carrying; obj != 0; obj = obj->next_content )
    {
        if ( obj->wear_loc != wear_none
             && ((id && obj->getID( ) == id) 
                 || (!id && obj_has_name( obj, arg, ch ))))
        {
            if (id || ++count == number)
                return obj;
        }
    }

    return 0;
}

Object * get_obj_here( Character *ch, const DLString & constArgument )
{
    char argument[MAX_INPUT_LENGTH];
    strcpy(argument, constArgument.c_str( ));
    return get_obj_here( ch, argument );
}
/*
 * Find an obj in the room or in inventory.
 */
Object *get_obj_here( Character *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        Object *obj;
        int count = 0;
        int number = number_argument( argument, arg );
        long long id = get_arg_id( argument );

        // At first look in inventory ...
        for ( obj = ch->carrying; obj != 0; obj = obj->next_content )
        {
                if ( obj->wear_loc == wear_none
                        && ( ch->can_see( obj ) || ch->can_hear( obj ) )
                        && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))) )
                {
                        if (id || ++count == number )
                                return obj;
                }
        }

        // ... then equipment
        for ( obj = ch->carrying; obj != 0; obj = obj->next_content )
        {
                if ( obj->wear_loc != wear_none
                        && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))) )
                {
                        if (id || ++count == number )
                                return obj;
                }
        }

        // ... then the floor.
        for ( obj = ch->in_room->contents; obj != 0; obj = obj->next_content )
        {
                if ( ( ch->can_see( obj ) || ch->can_hear( obj ) )
                        && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))) )
                {
                        if (id || ++count == number )
                                return obj;
                }
        }

        return 0;
}



/*
 * Find an obj in the world.
 */
Object *get_obj_world( Character *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    int number;
    int count;
    long long id = get_arg_id( argument );

    if ( ( obj = get_obj_here( ch, argument ) ) != 0 )
        return obj;

    number = number_argument( argument, arg );
    count  = 0;
    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if ( ch->can_see( obj ) && obj_has_name_or_id( obj, arg, ch, id ) )
        {
            if ( ++count == number )
                return obj;
        }

    }

    return 0;
}


/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, Object *list )
{
    Object *obj;
    int nMatch;

    nMatch = 0;
    for ( obj = list; obj != 0; obj = obj->next_content )
    {
        if ( obj->pIndexData == pObjIndex )
            nMatch++;
    }

    return nMatch;
}


/*
 * Find an obj in the room
 */
Object *get_obj_room( Character *ch, const DLString &arg )
{
    return get_obj_room( ch, arg.c_str( ) );
}
Object *get_obj_room( Character *ch, const char *cArgument )
{
    char arg[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    Object *obj;
    int count = 0, number;
    long long id = get_arg_id( cArgument );

    strcpy( argument, cArgument );
    number = number_argument( argument, arg );

    for ( obj = ch->in_room->contents; obj != 0; obj = obj->next_content ) {
        if (( ch->can_see( obj ) || ch->can_hear( obj ) )
              && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))) )
        {
            if (id || ++count == number )
                return obj;
        }
    }

    return NULL;
}

Object *get_obj_wear_victim( Character *victim, const DLString &cArg, Character *ch )
{
    char arg[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    Object *obj;
    int number;
    int count;
    long long id = get_arg_id( cArg );
    
    strcpy(argument, cArg.c_str( ));
    number = number_argument( argument, arg );
    count  = 0;

    for (obj = victim->carrying; obj; obj = obj->next_content )
        if (obj->wear_loc != wear_none
            && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, ch ))) 
            && ch->can_see( obj ))
        {
            if (id || ++count == number)
                return obj;
        }

    return NULL;
}

Object * get_obj_list_type( Character *ch, const DLString &cArg, int type, Object *list )
{
    char arg[MAX_INPUT_LENGTH];
    char argument[MAX_INPUT_LENGTH];
    Object *obj;
    int number;
    int count;
    
    strcpy(argument, cArg.c_str( ));
    number = number_argument( argument, arg );
    count  = 0;

    for (obj = list; obj; obj = obj->next_content )
        if (obj->item_type == type
            && obj_has_name( obj, arg, ch ) 
            && (ch->can_see( obj ) || ch->can_hear( obj )))
        {
            if (++count == number)
                return obj;
        }

    return NULL;
}

Object * get_obj_list_type( Character *ch, int type, Object *list )
{
    Object *obj;

    for (obj = list; obj; obj = obj->next_content)
        if (obj->item_type == type
            && (ch->can_see( obj ) || ch->can_hear( obj )))
            return obj;

    return NULL;
}

std::list<Object *> get_objs_list_type( Character *ch, int type, Object *list )
{
    Object *obj;
    std::list<Object *> result;    

    for (obj = list; obj; obj = obj->next_content)
        if (obj->item_type == type
            && (ch->can_see( obj ) || ch->can_hear( obj )))
            result.push_back(obj);

    return result;
}

Object * get_obj_room_type( Character *ch, int type )
{
    return get_obj_list_type( ch, type, ch->in_room->contents );
}

Object * get_obj_room_type( Room *room, int type )
{
    Object *obj;

    for (obj = room->contents; obj; obj = obj->next_content)
        if (obj->item_type == type)
            return obj;

    return NULL;
}

Object * get_obj_carry_type( Character *ch, int type )
{
    return get_obj_list_type( ch, type, ch->carrying );
}

Object * get_obj_room_vnum( Room *room, int vnum )
{
    Object *obj;

    for (obj = room->contents; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            return obj;

    return NULL;
}

Object * get_obj_list_vnum( Character *ch, int vnum, Object *list )
{
    Object *obj;

    for (obj = list; obj; obj = obj->next_content) 
        if (obj->pIndexData->vnum == vnum
            && (ch->can_see( obj ) || ch->can_hear( obj )))
            return obj;

    return NULL;
}

Object * get_obj_carry_vnum( Character *ch, int vnum )
{
    return get_obj_list_vnum( ch, vnum, ch->carrying );
}


PCharacter * get_player_world( Character *ch, const char *arg, bool fSeenOnly )
{
    Descriptor *d;

    for (d = descriptor_list; d != 0; d = d->next) {
        PCharacter *victim;
        
        if (d->connected != CON_PLAYING)
            continue;
        
        if(!d->character)
            continue;

        victim = d->character->getPC( );

        if (fSeenOnly && !ch->can_see( victim ))
            continue;

        if (!can_see_god(ch, victim))
            continue;

        if (!char_has_name( victim, arg ))
            continue;
        
        return victim;
    }

    return NULL;
}

Object * get_obj_world_unique( int vnum, Character *ch )
{
    Object *obj;
    
    for (obj = object_list; obj; obj = obj->next) 
        if (obj->pIndexData->vnum == vnum && obj->hasOwner( ch ))
            break;

    return obj;
}

std::list<Object *> get_objlist_world_unique( int vnum, Character *ch )
{
    Object *obj;
    std::list<Object *> list;
    
    for (obj = object_list; obj; obj = obj->next) 
        if (obj->pIndexData->vnum == vnum && obj->hasOwner( ch ))
            list.push_back( obj );

    return list;
}

Object * get_obj_room_unique( Room *room, int itype, Character *ch )
{
    for (Object *obj = room->contents; obj; obj = obj->next_content)
        if (obj->item_type == itype && obj->hasOwner( ch ))
            return obj;

    return NULL;
}

Object *find_pit_in_room(int roomVnum)
{
    Object *pit = 0;
    Room *pitRoom = get_room_instance(roomVnum);
    if (pitRoom)
        for (pit = pitRoom->contents;
             pit && !IS_PIT(pit);
             pit = pit->next_content)
            ;

    return pit;
}

Object *find_pit_for_obj(Object *obj)
{
    if (obj->item_type == ITEM_CORPSE_PC)
        return find_pit_in_room(obj->value3());
    
    return 0;
}

int count_obj_in_obj( Object *container )
{
    Object *objc;
    int count;

    for (count = 0, objc = container->contains; objc; objc = objc->next_content, count++)
        ;

    return count;
}

int count_obj_in_obj( Object *container, int itype )
{
    Object *objc;
    int count;

    for (count = 0, objc = container->contains; objc; objc = objc->next_content)
        if (objc->item_type == itype)
            count++;

    return count;
}


Object * get_obj_wear_carry( Character *ch, const DLString &cArgument, Character *looker )
{
    char arg[MAX_INPUT_LENGTH], argument[MAX_INPUT_LENGTH];
    Object *obj;
    int count = 0;
    int number;
    long long id = get_arg_id( cArgument );
    Character *whoCarries = ch;
    Character *whoSees = looker ? looker : ch;

    strcpy( argument, cArgument.c_str( ) );
    number = number_argument( argument, arg );

    for (obj = whoCarries->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc != wear_none
            && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, whoSees ))) )
        {
            if (id || ++count == number)
                return obj;
        }

    for (obj = whoCarries->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc == wear_none
                && (whoSees->can_see( obj ) || whoSees->can_hear( obj ))
                && ((id && obj->getID( ) == id) || (!id && obj_has_name( obj, arg, whoSees ))) )
        {
            if (id || ++count == number)
                return obj;
        }

    return 0;
}

Object * get_key_carry( Character *ch, int vnum )
{
    Object *key, *ring;
    
    if (( key = get_obj_carry_vnum( ch, vnum ) ))
        return key;

    for (ring = get_obj_carry_type( ch, ITEM_KEYRING );
         ring;
         ring = get_obj_list_type( ch, ITEM_KEYRING, ring->next_content ))
    {
        if (( key = get_obj_list_vnum( ch, vnum, ring->contains ) ))
            return key;
    }

    return NULL;
}

// Return true if immortal's config allows them to be seen.
bool can_see_god(Character *ch, Character *god)
{
    if (!ch) {
        return god->invis_level <= LEVEL_MORTAL 
                && god->incog_level <= LEVEL_MORTAL;
    }
    
    if (ch->get_trust() < god->invis_level)
        return false;
            
    if (ch->get_trust() < god->incog_level && ch->in_room != god->in_room)
        return false;

    return true;
}

bool mob_index_has_name( mob_index_data *pMob, const DLString &arg )
{
    return pMob->short_descr.matchesUnstrict(arg) || pMob->keyword.matchesUnstrict(arg);
}

bool obj_index_has_name( OBJ_INDEX_DATA *pObj, const DLString &arg )
{
    return pObj->short_descr.matchesUnstrict(arg) || pObj->keyword.matchesUnstrict(arg);
}

/** Return true if character can access object by the name. */
bool obj_has_name( Object *obj, const DLString &arg, Character *ch )
{
    // First try matching by obj keywords.
    if (is_name(arg.c_str(), String::toString(obj->getKeyword()).c_str()))
        return true;

    if (obj->getKeyword().matchesUnstrict(arg))
        return true;

    // Then try to match by short descr, all grammatical cases, no colours.
    if (obj->getShortDescr().matchesUnstrict(arg))
        return true;
        
    // No match.
    return false;
}

bool obj_has_name_or_id( Object *obj, const DLString &arg, Character *ch, long long id )
{    
    if (id && obj->getID( ) == id) 
        return true;

    if (!id && obj_has_name( obj, arg, ch ))
        return true;

    return false;
}                             

Profession * find_prof_unstrict( const DLString &className) 
{    
    for (int i = 0; i < professionManager->size( ); i++) {
        Profession *p = professionManager->find( i );
        if (!p->isValid( ) || !p->isPlayed( )) 
            continue;

        if (className.strPrefix( p->getName( ) )) {
            return p;
        }

        if (className.strPrefix( p->getRusName( ).ruscase( '1' ) )
                    || className.strPrefix( p->getRusName( ).ruscase( '5' )))
            return p;
    }
    return NULL;
}

bool text_match_with_highlight(const DLString &text, const DLString &args, ostringstream &matchBuf)
{
    static DLString highlight("{R");    
    stringstream lines(text);
    DLString line;            
    int num = 0;
    bool found = false;

    if (args.empty())
        return found;

    while (std::getline(lines, line, '\n')) {
        num++;
        if (line.find(args) != string::npos) {
            line.replaces(args, highlight + args + "{w");
            matchBuf << fmt(0, "{D%3d{x ", num) << line << "\r\n";
            found = true;
        }
    }

    return found;
}

NPCharacter * find_mob_with_behavior( Room *room, BehaviorReference &bhv )
{    
    for (Character* rch = room->people; rch != 0; rch = rch->next_in_room )
       if (rch->is_npc() && rch->getNPC()->pIndexData->behaviors.isSet(bhv))
          return rch->getNPC( );

    return NULL;
}



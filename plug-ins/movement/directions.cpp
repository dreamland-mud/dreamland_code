/* $Id$
 *
 * ruffina, 2004
 */
#include "directions.h"

#include "character.h"
#include "room.h"

#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

const char * extra_move_ru [] =
{
        "уш%1$Gло|ел|ла", "взобрал%1$Gось|ся|ась", "запрыгну%1$Gло|л|ла"
        , "бросил%1$Gось|ся|ась", "нырну%1$Gло|л|ла", "уплы%1$Gло|л|ла"
        , "всплы%1$Gло|л|ла", "протиснул%1$Gось|ся|ась", "улете%1$Gло|л|ла"
        , "спрыгну%1$Gло|л|ла", "сле%1$Gзло|з|зла", "спустил%1$Gось|ся|ась"
};

const char * extra_move_rp [] =
{
        "приш%1$Gло|ел|ла", "забрал%1$Gось|ся|ась", "запрыгну%1$Gло|л|ла"
        , "упа%1$Gло|л|ла", "донырну%1$Gло|л|ла", "приплы%1$Gло|л|ла"
        , "всплы%1$Gло|л|ла", "протиснул%1$Gось|ся|ась", "прилете%1$Gло|л|ла"
        , "спрыгну%1$Gло|л|ла", "сле%1$Gзло|з|зла", "спустил%1$Gось|ся|ась"
};

const char * extra_move_rt [] =
{
                "в", "на", "сквозь", "между", "над"
        , "через", "под", "с", "из", "со", "из под", "по"
};

char const extra_move_rtum [] =
{
        '4', '4', '4', '5', '5', '4', '5', '2', '2', '2', '2', '2', '2'
};

char const extra_move_rtpm [] =
{
        '2', '4', '4', '5', '5', '4', '5', '2', '2', '2', '2', '2', '2'
};

const struct direction_t dirs [] = {
    { DIR_NORTH, DIR_SOUTH, "north", "север",  "на север",  "с севера",  "на севере",  0, 0 },
    { DIR_EAST,  DIR_WEST,  "east",  "восток", "на восток", "с востока", "на востоке", 0, 0 },
    { DIR_SOUTH, DIR_NORTH, "south", "юг",     "на юг",     "с юга",     "на юге",     0, 0 },
    { DIR_WEST,  DIR_EAST,  "west",  "запад",  "на запад",  "с запада",  "на западе",  0, 0 },
    { DIR_UP,    DIR_DOWN,  "up",    "вверх",  "вверх",     "сверху",    "наверху",    "подняться", "^" },
    { DIR_DOWN,  DIR_UP,    "down",  "вниз",   "вниз",      "снизу",     "внизу",      "опуститься", "v"},
};

int direction_lookup( char c )
{
    char arg[2];
    arg[0] = c;
    arg[1] = 0;
    return direction_lookup( arg );
}

int direction_lookup( const char *arg )
{
    int door;
    
    if (!arg || !*arg)
        return -1;
        
    for (door = 0; door < DIR_SOMEWHERE; door++) {
        const direction_t &dir = dirs[door];

        if (arg[0] == dir.name[0] || arg[0] == dir.rname[0]) {
            if (arg[1] == 0) // neswup свюз
                return door;
            else if (arg[1] == dir.rname[1] && arg[2] == 0) // вв вн
                return door;
        }

        if (dir.rname_extra_1 && arg[1] == 0 && arg[0] == dir.rname_extra_1[0]) // о, п
            return door;

        if (dir.rname_extra_2 && arg[1] == 0 && arg[0] == dir.rname_extra_2[0]) // ^, v
            return door;

        if (!str_prefix( arg, dir.name ))
            return door;

        if (!str_prefix( arg, dir.rname ))
            return door;

        if (dir.rname_extra_1 && !str_prefix( arg, dir.rname_extra_1 ))
            return door;
    }
    
    return -1;
}

int find_exit( Character *ch, const char *arg, int flags )
{
    EXIT_DATA *pexit;
    int door = direction_lookup( arg );

    if (door < 0) {
        for (int d = 0; d < DIR_SOMEWHERE; d++) {
            pexit = ch->in_room->exit[d];

            if (!pexit || !pexit->u1.to_room)
                continue;

            if (!ch->can_see( pexit ) && IS_SET(flags, FEX_NO_INVIS))
                continue;

            if (!IS_SET( pexit->exit_info, EX_ISDOOR ) && IS_SET(flags, FEX_DOOR))
                continue;

            if (pexit->keyword && is_name( arg, pexit->keyword ))
                return d;
            
            if (pexit->short_descr 
                  && (is_name(arg, russian_case(pexit->short_descr, '1').c_str())
                      || is_name(arg, russian_case(pexit->short_descr, '4').c_str())))
                return d;

            if (!str_prefix(arg, "дверь") || !str_prefix(arg, "door"))
                return d;
        }

        if (IS_SET(flags, FEX_VERBOSE))
            act( "Ты не видишь $T здесь.", ch, 0, arg, TO_CHAR );
        
        return door;
    }
    
    pexit = ch->in_room->exit[door];
    
    if ((!pexit || !pexit->u1.to_room) && IS_SET(flags, FEX_NO_EMPTY)) {
        if (IS_SET(flags, FEX_VERBOSE))
            act( "Ты не видишь выхода $T отсюда.", ch, 0, dirs[door].leave, TO_CHAR);

        return -1;
    }

    if (pexit && !ch->can_see( pexit ) && IS_SET(flags, FEX_NO_INVIS)) {
        if (IS_SET(flags, FEX_VERBOSE))
            act( "Ты не видишь выхода $T отсюда.", ch, 0, dirs[door].leave, TO_CHAR);

        return -1;
    }

    if ((!pexit || !IS_SET(pexit->exit_info, EX_ISDOOR)) && IS_SET(flags, FEX_DOOR)) {
        if (IS_SET(flags, FEX_VERBOSE))
            act( "Ты не видишь двери $T отсюда.", ch, 0, dirs[door].leave, TO_CHAR);

        return -1;
    }

    return door;
}

EXTRA_EXIT_DATA * get_extra_exit ( const char * name,EXTRA_EXIT_DATA * list )
{
        for( ; list != 0; list = list->next )
        {
                if ( is_name( (char *) name , list->keyword ) )
                        return list;
        }

        return 0;
}

const char * direction_doorname(EXIT_DATA *pexit)
{
    if (!pexit || !pexit->short_descr || !pexit->short_descr[0])
        return "дверь";
    return pexit->short_descr;
}

exit_data *direction_reverse(Room *room, int door)
{
    Room *to_room;       
    EXIT_DATA *pexit = room->exit[door], *pexit_rev;

    if (!pexit)
        return 0;

    if (!(to_room = pexit->u1.to_room))
        return 0;

    if (!(pexit_rev = to_room->exit[dirs[door].rev]))
        return 0;

    if (pexit_rev->u1.to_room != room)
        return 0;

    return pexit_rev;
}

Room * direction_target(Room *room, int door)
{
    if (!room->exit[door])
        return NULL;
    return room->exit[door]->u1.to_room;
}


/** 
 * Split into arguments 'n.victim', 'north victim', 'n victim', 'north.victim'.
 * Return true if first word is a valid direction name.
 */
bool direction_range_argument(const DLString &cargs, DLString &argDoor, DLString &argVict, int &door)
{
    unsigned int i;

    argDoor = "";
    argVict = cargs;
    door = -1;

    for (i = 0; i < cargs.size() && cargs.at(i) != '.' && cargs.at(i) != ' '; i++)
        ;

    if (i == 0 || i == cargs.size() || i == cargs.size() - 1)
        return false;

    argDoor = cargs.substr(0, i);
    if ((door = direction_lookup(argDoor.c_str())) < 0) {
        argDoor = "";
        return false;
    }

    argVict = cargs.substr(i+1);
    return true;
}
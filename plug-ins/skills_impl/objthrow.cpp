
/* $Id: objthrow.cpp,v 1.1.2.5 2008/04/14 20:12:37 rufina Exp $
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
#include <string.h>
#include "objthrow.h"

#include "skill.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "fight.h"
#include "damage.h"
#include "loadsave.h"
#include "wearloc_utils.h"
#include "magic.h"
#include "clanreference.h"
#include "interp.h"
#include "merc.h"

#include "act.h"
#include "def.h"
#include "l10n.h"

Character * find_char( Character *ch, const char *cArgument, int door, int *range, ostringstream &errbuf )
{
    char argument[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    EXIT_DATA *pExit, *bExit;
    Room *dest_room, *back_room;
    Character *target;
    int number = 0, opdoor;
    
    strcpy( argument, cArgument );
    number = number_argument(argument,arg);
    dest_room = ch->in_room;

    // Look for target in the same room only if a non-ranged lookup is requested.
    if (door == -1 && (target = get_char_room(ch,dest_room,arg,&number)) != 0)
        return target;

    opdoor = dirs[door].rev;

    while (*range > 0)
    {
        *range = *range - 1;
        /* find target room */
        back_room = dest_room;

        if ( (pExit = dest_room->exit[door]) == 0
            || !ch->can_see( pExit )
            || (dest_room = pExit->u1.to_room ) == 0
            || IS_SET(pExit->exit_info,EX_CLOSED) )
            break;

        if ( (bExit = dest_room->exit[opdoor]) == 0
            || bExit->u1.to_room != back_room)
        {
            errbuf << lmsg(viewerLang(ch),
                           "You can't reach them through a one-way passage.",
                           "Ты не сможешь добраться до них через односторонний проход.",
                           "Ти не зможеш дістатися до них через односторонній прохід.") << endl;
            return 0;
        }

        if ((target = get_char_room(ch,dest_room,arg,&number)) != 0 )
            return target;
    }
    
    errbuf << fmt(0, lmsg(viewerLang(ch),
                          "Looking %s, you see no one by that name.",
                          "Ты не видишь %s никого с таким именем.",
                          "Ти не бачиш %s нікого з таким ім'ям."),
                  direction_word(viewerLang(ch), door, DIR_CASE_AT)) << endl;
    return 0;
}
        



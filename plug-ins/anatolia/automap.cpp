/* $Id$
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
 *    Zadvinsky Aleksandr  {Kiddy}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "commandtemplate.h"
#include "character.h"
#include "room.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"
#include "act_move.h"


#define MAX_MAP 72
#define MAX_MAP_DIR 4

const char *localMap[MAX_MAP][MAX_MAP];
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0,-1} };

void MapArea(Room *room, Character *ch, int x, int y, int
             min, int max)
{
    Room *prospect_room;
    EXIT_DATA *pexit;
    int door;

    switch (room->sector_type){
    case SECT_INSIDE:                localMap[x][y]="{W%";                break;
    case SECT_CITY:                localMap[x][y]="{W#";                break;
    case SECT_FIELD:                localMap[x][y]="{G\"";                break;
    case SECT_FOREST:                localMap[x][y]="{g@";                break;
    case SECT_HILLS:                localMap[x][y]="{G^";                break;
    case SECT_MOUNTAIN:                localMap[x][y]="{y^";                break;
    case SECT_WATER_SWIM:        localMap[x][y]="{B~";                break;
    case SECT_WATER_NOSWIM:        localMap[x][y]="{b~";                break;
    case SECT_UNUSED:                localMap[x][y]="{DX";                break;
    case SECT_AIR:                localMap[x][y]="{C:";                break;
    case SECT_DESERT:                localMap[x][y]="{Y=";                break;
    default:                         localMap[x][y]="{yo";
    }

    for ( door = 0; door < MAX_MAP_DIR; door++ )
    {
        if (
            (pexit = room->exit[door]) != NULL
            &&   ch->can_see( pexit )
            &&   !IS_SET(pexit->exit_info, EX_CLOSED)
            )
        {                        

            prospect_room = pexit->u1.to_room;

            if ( prospect_room->exit[dirs[door].rev] &&
                prospect_room->exit[dirs[door].rev]->u1.to_room!=room)
            {                        /* if not two way */
                if ((prospect_room->sector_type==SECT_CITY)
                    ||  (prospect_room->sector_type==SECT_INSIDE))
                    localMap[x][y]="{W@";
                else
                    localMap[x][y]="{D?";
                return;
            } /* end two way */

            if ((x<=min)||(y<=min)||(x>=max)||(y>=max)) return;
            if (localMap[x+offsets[door][0]][y+offsets[door][1]]==NULL) {
                MapArea ( pexit->u1.to_room,ch,
                         x+offsets[door][0], y+offsets[door][1],min,max);
            }

        } /* end if exit there */
    }
    return;
}

void ShowMap( Character *ch, int min, int max)
{
    int x,y;

    for (x = min; x < max; ++x)
    {
        for (y = min; y < max; ++y)
        {
            if (localMap[x][y]==NULL) ch->send_to(" ");                
            else                 ch->send_to(localMap[x][y]);         
        }
        ch->send_to("\n\r");
    }
    ch->send_to("{x\n\r");

    return;
}

CMDRUNP( map )
{
    int size,center,x,y,min,max;
    char arg1[10];

    one_argument( argument, arg1 );
    size = atoi (arg1);

    size=URANGE(7,size,72);
    center=MAX_MAP/2;

    min = MAX_MAP/2-size/2;
    max = MAX_MAP/2+size/2;

    for (x = 0; x < MAX_MAP; ++x)
        for (y = 0; y < MAX_MAP; ++y)
            localMap[x][y]=NULL;

    MapArea(ch->in_room, ch, center, center, min, max);

    localMap[center][center]="{R*";        
    ShowMap (ch, min, max); 

    return;
}

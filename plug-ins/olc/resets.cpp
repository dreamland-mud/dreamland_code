
#include "character.h"
#include "room.h"
#include "wearlocation.h"
#include "../anatolia/handler.h"
#include "olcstate.h"
#include "olc.h"
#include "directions.h"
#include "olcflags.h"
#include "act.h"
#include "websocketrpc.h"
#include "comm.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

static void display_resets(Character * ch)
{
    ostringstream buf;
    RoomIndexData *pRoom;
    MOB_INDEX_DATA *pMob = NULL;

    pRoom = ch->in_room->pIndexData;

    buf << "Resets for room [{C" << web_cmd(ch, "redit $1", pRoom->vnum) << "{x] {W" << pRoom->name << "{x:" << endl;

    for (unsigned int iReset = 0; iReset < pRoom->resets.size(); iReset++) {
        OBJ_INDEX_DATA *pObj;        
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        RoomIndexData *pRoomIndex;
        RESET_DATA *pReset = pRoom->resets[iReset];
        char cmd = pReset->command;
        DLString line;

        char numColor = 'D';
        if (cmd == 'M' || cmd == 'O' || cmd == 'R' || cmd == 'D')
            numColor = 'W';
        buf << fmt(0, "{%c%2d){x ", numColor, iReset+1);

        switch (cmd) {
        default:
            buf << fmt(0, "Bad reset command: %c.", pReset->command);
            break;
        case 'M':
            if (!(pMobIndex = get_mob_index(pReset->arg1))) {
                buf << fmt(0, "Load Mobile - Bad Mob %d\n\r", pReset->arg1);
                continue;
            }
            if (!(pRoomIndex = get_room_index(pReset->arg3))) {
                buf << fmt(0, "Load Mobile - Bad Room %d\n\r", pReset->arg3);
                continue;
            }

            pMob = pMobIndex;
            line = "M [" + web_cmd(ch, "medit $1", "%5d") + "] %-24.24s %2d-%2d {x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1, 
                      russian_case(pMob->short_descr, '1').colourStrip( ).c_str( ),
                      pReset->arg2, 
                      pReset->arg4);
            break;

        case 'O':
            if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf << fmt(0, "Load Object - Bad Object %d\n\r", pReset->arg1);
                continue;
            }
            pObj = pObjIndex;
            if (!(pRoomIndex = get_room_index(pReset->arg3))) {
                buf << fmt(0, "Load Object - Bad Room %d\n\r", pReset->arg3);
                continue;
            }

            line = "O [" + web_cmd(ch, "oedit $1", "%5d") +"] %-24.24s{x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1, 
                      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ));
            break;

        case 'P':
            if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf << fmt(0, "Put Object - Bad Object %d\n\r", pReset->arg1);
                continue;
            }

            pObj = pObjIndex;

            if (!(pObjToIndex = get_obj_index(pReset->arg3))) {
                buf << fmt(0, "Put Object - Bad To Object %d\n\r", pReset->arg3);
                continue;
            }

            line = "          [" + web_cmd(ch, "oedit $1", "%5d") + "] %-24.24s %2d-%2d {x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1,
                      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                      pReset->arg2,
                      pReset->arg4);
            break;

        case 'G':
        case 'E':
            if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf << fmt(0, "Give/Equip Object - Bad Object %d\n\r", pReset->arg1);
                continue;
            }

            pObj = pObjIndex;

            if (!pMob) {
                buf << fmt(0, "Give/Equip Object - No Previous Mobile\n\r");
                break;
            }

            line = "          [" + web_cmd(ch, "oedit $1", "%5d") +"] %-24.24s %-8.8s {x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1,
                      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                      (cmd == 'G') ?
                          wear_none.getName( ).c_str( )
                          : wearlocationManager->find( pReset->arg3 )->getName( ).c_str( ));
            break;

        case 'D':
            pRoomIndex = get_room_index(pReset->arg1);
            buf << fmt(0, "D [%5d] %s door reset to %s{x\n\r",
                      pReset->arg1,
                      DLString(dirs[pReset->arg2].name).capitalize( ).c_str( ),
                      door_resets_table.name(pReset->arg3).c_str());
            break;

        case 'R':
            if (!(pRoomIndex = get_room_index(pReset->arg1))) {
                buf << fmt(0, "Randomize Exits - Bad Room %d\n\r", pReset->arg1);
                continue;
            }

            buf << fmt(0, "R [%5d] Exits are randomized{x\n\r",
                      pReset->arg1);
            break;
        }
    }

    page_to_char(buf.str().c_str(), ch);
}

static void add_reset(RoomIndexData * room, RESET_DATA * pReset, int index)
{
    if (index < 0 || index >= (int)room->resets.size()) {
        room->resets.push_back(pReset);
        return;
    }

    room->resets.insert(room->resets.begin() + index, pReset);
}

CMD(resets, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online resets editor.")
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char arg4[MAX_INPUT_LENGTH];
    char arg5[MAX_INPUT_LENGTH];
    char arg6[MAX_INPUT_LENGTH];
    char arg7[MAX_INPUT_LENGTH];
    RESET_DATA *pReset = NULL;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    argument = one_argument(argument, arg3);
    argument = one_argument(argument, arg4);
    argument = one_argument(argument, arg5);
    argument = one_argument(argument, arg6);
    argument = one_argument(argument, arg7);
    
    if (!OLCState::can_edit( ch, ch->in_room->vnum )) {
        stc("Resets: Invalid security for editing this area.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0') {
        if (!ch->in_room->pIndexData->resets.empty()) {
            display_resets(ch);
        }
        else
            stc("No resets in this room.\n\r", ch);
    }

    if (is_number(arg1)) {
        RoomIndexData *pRoom = ch->in_room->pIndexData;

        if (!str_cmp(arg2, "delete")) {
            int insert_loc = atoi(arg1);

            if (!pRoom->resets.empty()) {
                stc("No resets in this area.\n\r", ch);
                return;
            }

            insert_loc--;
            if (insert_loc < 0 || insert_loc >= (int)pRoom->resets.size()) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            pRoom->resets.erase(pRoom->resets.begin() + insert_loc);
            ddeallocate(pReset);
            SET_BIT(ch->in_room->areaIndex()->area_flag, AREA_CHANGED);
            stc("Reset deleted.\n\r", ch);
        }
        else if ((!str_cmp(arg2, "mob") && is_number(arg3))
                 || (!str_cmp(arg2, "obj") && is_number(arg3))) {
            if (!str_cmp(arg2, "mob")) {
                pReset = new reset_data();
                pReset->command = 'M';
                if (get_mob_index(is_number(arg3) ? atoi(arg3) : 1) == NULL) {
                    stc("Монстр не существует.\n\r", ch);
                    return;
                }
                pReset->arg1 = atoi(arg3);
                pReset->arg2 = is_number(arg4) ? atoi(arg4) : 1;        /* Max # */
                pReset->arg3 = ch->in_room->vnum;
                pReset->arg4 = is_number(arg5) ? atoi(arg5) : 1;        /* Min # */
            }
            else if (!str_cmp(arg2, "obj")) {
                pReset = new reset_data();
                pReset->arg1 = atoi(arg3);
                if (!str_prefix(arg4, "inside")) {
                    pReset->command = 'P';
                    pReset->arg2 = 0;
                    if ((get_obj_index(is_number(arg5) ? atoi(arg5) : 1))->item_type != ITEM_CONTAINER) {
                        stc("Предмет не является контейнером.\n\r", ch);
                        return;
                    }
                    pReset->arg2 = is_number(arg6) ? atoi(arg6) : 1;
                    pReset->arg3 = is_number(arg5) ? atoi(arg5) : 1;
                    pReset->arg4 = is_number(arg7) ? atoi(arg7) : 1;
                }
                else if (!str_cmp(arg4, "room")) {
                    pReset = new reset_data();
                    pReset->command = 'O';
                    if (get_obj_index(atoi(arg3)) == NULL) {
                        stc("Предметов с таким номером не существует.\n\r", ch);
                        return;
                    }
                    pReset->arg1 = atoi(arg3);
                    pReset->arg2 = 0;
                    pReset->arg3 = ch->in_room->vnum;
                    pReset->arg4 = 0;
                }
                else {
                    Wearlocation *wl;
                    
                    if (!( wl = wearlocationManager->findExisting( arg4 ) )) {
                        stc("Resets: '? wear-loc'\n\r", ch);
                        return;
                    }
                    pReset = new reset_data();
                    if (get_obj_index(atoi(arg3)) == NULL) {
                        stc("Предметов с таким номером не существует.\n\r", ch);
                        return;
                    }
                    pReset->arg1 = atoi(arg3);
                    pReset->arg3 = wl->getIndex( );
                    if (pReset->arg3 == wear_none)
                        pReset->command = 'G';
                    else
                        pReset->command = 'E';
                }
            }
            add_reset(ch->in_room->pIndexData, pReset, atoi(arg1));
            SET_BIT(ch->in_room->areaIndex()->area_flag, AREA_CHANGED);
            stc("Reset added.\n\r", ch);
        }
        else {
            stc("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
            stc("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch);
            stc("        RESET <number> OBJ <vnum> room\n\r", ch);
            stc("        RESET <number> MOB <vnum> [max # area] [max # room]\n\r", ch);
            stc("        RESET <number> DELETE\n\r", ch);
        }
    }
}


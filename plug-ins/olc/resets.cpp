
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
#include "weapontier.h"
#include "comm.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

static DLString one_reset_flag(Character *ch, bitstring_t flag, int iReset, RESET_DATA *pReset)
{
    ostringstream buf;
    DLString fname = reset_flags.names(flag);

    buf << "{" << (pReset->flags.isSet(flag) ? "g" : "D") << fname << " ["
        << web_cmd(
                ch, 
                "resets " + DLString(iReset) + " flag " + fname,
                pReset->flags.isSet(flag) ? "X" : "-")
        << "]{x";

    return buf.str();
}

static DLString show_reset_flags(Character *ch, int iReset, RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex)
{
    ostringstream buf;

    if (pObjIndex->item_type != ITEM_WEAPON) {
        return "{g" + pReset->flags.names();
    }

    buf << one_reset_flag(ch, RESET_RAND_STAT, iReset, pReset)
        << " "
        << one_reset_flag(ch, RESET_RAND_ALL, iReset, pReset)
        << " {";

    if (pReset->flags.isSet(RESET_RAND_STAT|RESET_RAND_ALL))
        buf << "g";
    else
        buf << "D";

    buf << "{Iw<m i='reset " << iReset << " tier ' c='";
    for (int t = BEST_TIER; t <= WORST_TIER; t++) 
        buf << "$ " << t << ", ";
    buf << "'>{Ix[tier " << pReset->minTier << "]{Iw</m>{Ix";

    return buf.str();
}


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
            pObj = 0;

            line = "M [" + web_cmd(ch, "medit $1", "%5d") + "] {G%-24.24s{x %2d-%2d {g%s{x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1, 
                      russian_case(pMob->short_descr, '1').colourStrip( ).c_str( ),
                      pReset->arg2, 
                      pReset->arg4,
                      pReset->flags.names().c_str());
            break;

        case 'O':
            if (!(pObjIndex = get_obj_index(pReset->arg1))) {
                buf << fmt(0, "Load Object - Bad Object %d\n\r", pReset->arg1);
                continue;
            }

            pObj = pObjIndex;
            pMob = 0;

            if (!(pRoomIndex = get_room_index(pReset->arg3))) {
                buf << fmt(0, "Load Object - Bad Room %d\n\r", pReset->arg3);
                continue;
            }

            line = "O [" + web_cmd(ch, "oedit $1", "%5d") +"] {G%-24.24s{x %s{x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1, 
                      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                      show_reset_flags(ch, iReset+1, pReset, pObj).c_str());
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

            if (pMob)
                buf << "        ";

            line = "  [" + web_cmd(ch, "oedit $1", "%5d") + "] %-24.24s %2d-%2d %s{x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1,
                      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                      pReset->arg2,
                      pReset->arg4,
                      show_reset_flags(ch, iReset+1, pReset, pObj).c_str());
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

            line = "  [" + web_cmd(ch, "oedit $1", "%5d") +"] %-24.24s {y%-8.8s{x %s{x\n\r";
            buf << fmt(0, line.c_str(),
                      pReset->arg1,
                      russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                      (cmd == 'G') ?
                          wear_none.getName( ).c_str( )
                          : wearlocationManager->find( pReset->arg3 )->getName( ).c_str( ),
                      show_reset_flags(ch, iReset+1, pReset, pObj).c_str());
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

static int find_reset(RoomIndexData *pRoom, const char *arg)
{
    int insert_loc = atoi(arg);

    if (pRoom->resets.empty())
        return -1;

    insert_loc--;
    if (insert_loc < 0 || insert_loc >= (int)pRoom->resets.size())
        return -1;

    return insert_loc;
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
    
    if (!OLCState::can_edit( ch, ch->in_room->vnum )) {
        stc("Resets: Invalid security for editing this area.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0') {
        if (!ch->in_room->pIndexData->resets.empty())
            display_resets(ch);
        else
            stc("No resets in this room.\n\r", ch);

        return;
    }

    if (is_number(arg1)) {
        RoomIndexData *pRoom = ch->in_room->pIndexData;

        if (!str_cmp(arg2, "delete")) {
            int insert_loc = find_reset(pRoom, arg1);

            if (insert_loc < 0) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            pRoom->resets.erase(pRoom->resets.begin() + insert_loc);
            ddeallocate(pReset);
            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            stc("Reset deleted.\n\r", ch);
            return;
        }

        if (arg_oneof(arg2, "flag", "флаг")) {
            int insert_loc = find_reset(pRoom, arg1);

            if (insert_loc < 0) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            bitnumber_t flags = reset_flags.bitstring(argument, false);
            if (flags == NO_FLAG) {
                stc("Flags no found, see {y{hcolchelp reset_flags{x.\r\n", ch);
                return;
            }

            pReset->flags.toggleBit(flags);
            if (pReset->flags.isSet(RESET_RAND_STAT|RESET_RAND_ALL) && pReset->minTier <= 0)
                pReset->minTier = pReset->maxTier = WORST_TIER;

            if (pReset->flags.getValue() != 0)
                ptc(ch, "Flags for reset {W%d{x toggled to {g%s{x.\r\n", insert_loc+1, pReset->flags.names().c_str());
            else
                ptc(ch, "Flags for reset {W%d{x cleared.\r\n", insert_loc+1);

            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            return;
        }

        if (arg_oneof(arg2, "tier", "тиер")) {
            int insert_loc = find_reset(pRoom, arg1);

            if (insert_loc < 0) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            int tier = atoi(argument);
            if (tier < BEST_TIER || tier > WORST_TIER) {
                ptc(ch, "Tier must be a number from %d to %d.\r\n", BEST_TIER, WORST_TIER);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            pReset->minTier = pReset->maxTier = tier;
            ptc(ch, "Tier for reset {W%d{x set to {g%d{x.\r\n", insert_loc+1, tier);
            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            return;
        }

        argument = one_argument(argument, arg3);
        argument = one_argument(argument, arg4);
        argument = one_argument(argument, arg5);
        argument = one_argument(argument, arg6);
        argument = one_argument(argument, arg7);

        if ((!str_cmp(arg2, "mob") && is_number(arg3))
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
            return;
        }
    }


    stc("Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch);
    stc("        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch);
    stc("        RESET <number> OBJ <vnum> room\n\r", ch);
    stc("        RESET <number> MOB <vnum> [max # area] [max # room]\n\r", ch);
    stc("        RESET <number> DELETE\n\r", ch);
    stc("        RESET <number> FLAG <flags to toggle>\n\r", ch);
    stc("        RESET <number> TIER <1..5>\n\r", ch);
}


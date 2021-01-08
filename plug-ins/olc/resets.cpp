
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "wearlocation.h"
#include "../anatolia/handler.h"
#include "olcstate.h"
#include "olc.h"
#include "directions.h"
#include "olcflags.h"
#include "act.h"
#include "websocketrpc.h"
#include "weapontier.h"
#include "interp.h"
#include "comm.h"
#include "update_areas.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(sheath);

static char get_item_colour(OBJ_INDEX_DATA *pObj, RESET_DATA *pReset)
{
    int bestTier = pReset->bestTier;
    if (bestTier <= 0)
        bestTier = get_item_tier(pObj);
    
    if (bestTier <= 0)
        return 'w';
        
    weapon_tier_t &tier = weapon_tier_table[bestTier-1];
    DLString clr = tier.colour;
    if (clr.empty())
        return 'w';
    else
        return clr.at(0);
}

static DLString show_reset_wearloc(Character *ch, int iReset, RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, RoomIndexData *pRoom)
{
    ostringstream buf;
    Wearlocation *wearloc = 
        (pReset->command == 'E' ? wearlocationManager->find(pReset->arg3) : wear_none.getElement());
    DLString id = "at " + DLString(pRoom->vnum) + " reset " + DLString(iReset) + " wear";
    DLString label = "{y[" + wearloc->getName() + "]";

    StringList commands;
    for (int i = 0; i < wearlocationManager->size(); i++) {
        const DLString &wname = wearlocationManager->find(i)->getName();
        DLString exclude = "tat_";
        if (!exclude.strPrefix(wname))
            commands.push_back("$ " + wname);
    }

    buf << web_menu(commands, id, label);        
    return buf.str();
}

static DLString show_reset_rand(Character *ch, int iReset, RESET_DATA *pReset, OBJ_INDEX_DATA *pObjIndex, RoomIndexData *pRoom)
{
    ostringstream buf;

    if (pObjIndex->item_type != ITEM_WEAPON)
        return DLString::emptyString;

    {
        DLString id = "at " + DLString(pRoom->vnum) + " reset " + DLString(iReset) + " rand";
        StringList commands;
        commands.push_back("$ normal");
        commands.push_back("$ rand_stat");
        commands.push_back("$ rand_all");
        DLString label = "{y[" + pReset->rand.name() + "]";

        buf << web_menu(commands, id, label) << " ";
    }

    if (pReset->rand != RAND_NONE || pObjIndex->properties.count("random") > 0) {
        DLString id = "at " + DLString(pRoom->vnum) + " reset " + DLString(iReset) + " tier";

        StringList commands;
        for (int t = BEST_TIER + 1; t <= WORST_TIER; t++)
            commands.push_back("$ " + DLString(t));

        DLString label = "{y[tier " + DLString(pReset->bestTier) + "]";

        buf << web_menu(commands, id, label);
    }

    return buf.str();
}

struct filter_t {
    filter_t() 
    {
        vnum = rand = -1;
        pObj = 0;
        pMob = 0;
    }

    filter_t(int vnum, int rand) : filter_t()
    {
        this->vnum = vnum;
        this->rand = rand;
        if (vnum > 0) {
            pObj = get_obj_index(vnum);
            /*pMob = get_mob_index(vnum); -- uncomment to filter by mob vnum */
        } 
    }

    bool matches(RESET_DATA *pReset) const 
    {
        if (rand >= 0 && pReset->rand != rand)
            return false;

        if (!pMob && !pObj)
            return true;

        switch (pReset->command) {
        case 'M':
            if (pMob && pReset->arg1 == vnum)
                return true;
            break;

        case 'G':
        case 'E':
        case 'O':
        case 'P':
            if (pObj && pReset->arg1 == vnum)
                return true;
            break;
        }

        return false;
    }

    int vnum;
    int rand;
    OBJ_INDEX_DATA *pObj;
    MOB_INDEX_DATA *pMob;
};

static void display_resets(ostringstream &result, Character * ch, RoomIndexData *pRoom, const filter_t &f)
{
    ostringstream buf;
    MOB_INDEX_DATA *pMob = NULL;
    bool found = false;

    for (unsigned int iReset = 0; iReset < pRoom->resets.size(); iReset++) {
        OBJ_INDEX_DATA *pObj;        
        MOB_INDEX_DATA *pMobIndex;
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_INDEX_DATA *pObjToIndex;
        RoomIndexData *pRoomIndex;
        RESET_DATA *pReset = pRoom->resets[iReset];
        char cmd = pReset->command;
        DLString line;
        bool matches = f.matches(pReset);

        if (matches) {
            char numColor = 'D';
            if (cmd == 'M' || cmd == 'O' || cmd == 'R' || cmd == 'D')
                numColor = 'W';
            buf << fmt(0, "{%c%2d{x ", numColor, iReset+1);
        }

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

            if (matches) {
                found = true;
                line = "M[" + web_cmd(ch, "medit $1", "%5d") + "] %-24.24s{x %2d-%2d {g%s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1, 
                        russian_case(pMob->short_descr, '1').colourStrip( ).c_str( ),
                        pReset->arg2, 
                        pReset->arg4,
                        pReset->flags.names().c_str());
            }
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

            if (matches) {
                found = true;
                line = "O[" + web_cmd(ch, "oedit $1", "%5d") +"] {%c%-24.24s{x %s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1, 
                        get_item_colour(pObj, pReset),
                        russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                        show_reset_rand(ch, iReset+1, pReset, pObj, pRoom).c_str());
            }
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

            if (matches) {
                if (pMob)
                    buf << "        ";

                found = true;
                line = "         [" + web_cmd(ch, "oedit $1", "%5d") + "] {%c%-24.24s %2d-%2d %s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1,
                        get_item_colour(pObj, pReset),
                        russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                        pReset->arg2,
                        pReset->arg4,
                        show_reset_rand(ch, iReset+1, pReset, pObj, pRoom).c_str());
            }
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

            if (matches) {
                found = true;
                line = "         [" + web_cmd(ch, "oedit $1", "%5d") +"] {%c%-24.24s %s %s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1,
                        get_item_colour(pObj, pReset),
                        russian_case(pObj->short_descr, '1').colourStrip( ).c_str( ),
                        show_reset_wearloc(ch, iReset+1, pReset, pObj, pRoom).c_str(),
                        show_reset_rand(ch, iReset+1, pReset, pObj, pRoom).c_str());
            }
            break;

        case 'D':
            if (matches) {
                found = true;
                pRoomIndex = get_room_index(pReset->arg1);
                buf << fmt(0, "D [%5d] %s door reset to %s{x\n\r",
                        pReset->arg1,
                        DLString(dirs[pReset->arg2].name).capitalize( ).c_str( ),
                        door_resets_table.name(pReset->arg3).c_str());
            }
            break;

        case 'R':
            if (!(pRoomIndex = get_room_index(pReset->arg1))) {
                buf << fmt(0, "Randomize Exits - Bad Room %d\n\r", pReset->arg1);
                continue;
            }

            if (matches) {
                found = true;
                buf << fmt(0, "R [%5d] Exits are randomized{x\n\r",
                        pReset->arg1);
            }
            break;
        }
    }

    if (found) {        
        result << "Resets for room [{C" << web_cmd(ch, "redit $1", pRoom->vnum) << "{x] {W" << pRoom->name << "{x:" << endl;
        result << buf.str();
    }
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
    
    if (!OLCState::can_edit( ch, ch->in_room->vnum )) {
        stc("Resets: Invalid security for editing this area.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0') {
        if (!ch->in_room->pIndexData->resets.empty()) {
            ostringstream buf;
            display_resets(buf, ch, ch->in_room->pIndexData, filter_t());
            ch->send_to(buf);
        } else {
            stc("No resets in this room.\n\r", ch);
        }

        return;
    }

    // Syntax: search <vnum>, search rand_stat, search <vnum> rand_stat
    if (arg_oneof(arg1, "search", "поиск")) {
        ostringstream buf;
        DLString args = argument;
        DLString argVnum = args.getOneArgument();
        DLString argRand = argVnum;
        int vnum = 0;
        int rand = -1;

        if (argVnum.empty()) {
            stc("Please specify item vnum or rand value.\r\n", ch);
            return;
        }

        if (argVnum.isNumber()) {
            argRand = args.getOneArgument();
            vnum = argVnum.toInt();           
        }
        
        if (!argRand.empty()) {
            rand = rand_table.value(argRand, false);

            if (rand == NO_FLAG) {
                stc("Usage: resets search [<vnum>] [normal|rand_stat|rand_all]\r\n", ch);
                return;
            }

            if (vnum == 0 && rand == RAND_NONE) {
                stc("Output for 'normal' resets is too long, specify item vnum or rand.\r\n", ch);
                return;
            }
        }

        filter_t filter(vnum, rand);
        if (vnum > 0 && !filter.pObj && !filter.pMob) {
            stc("Can't find an item with this vnum.\r\n", ch);
            return;
        }

        for (auto &r: roomIndexMap)
            display_resets(buf, ch, r.second, filter);

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    argument = one_argument(argument, arg2);

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
                stc("Flags not found, see {y{hcolchelp reset_flags{x.\r\n", ch);
                return;
            }

            pReset->flags.toggleBit(flags);

            if (pReset->flags.getValue() != 0)
                ptc(ch, "Flags for reset {W%d{x toggled to {g%s{x.\r\n", insert_loc+1, pReset->flags.names().c_str());
            else
                ptc(ch, "Flags for reset {W%d{x cleared.\r\n", insert_loc+1);

            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            return;
        }

        argument = one_argument(argument, arg3);
        argument = one_argument(argument, arg4);

        if (arg_oneof(arg2, "rand", "ранд")) {
            int insert_loc = find_reset(pRoom, arg1);
            if (insert_loc < 0) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            int rand = rand_table.value(arg3, false);
            if (rand == NO_FLAG) {
                stc("Rand value no found, see {y{hcolchelp rand_table{x.\r\n", ch);
                return;
            }

            pReset->rand.setValue(rand);            
            ptc(ch, "Rand value for reset {W%d{x set to {g%s{x.\r\n", insert_loc+1, pReset->rand.name().c_str());

            if (pReset->bestTier <= 0) {
                pReset->bestTier = DEFAULT_TIER;
                ptc(ch, "Best tier for reset {W%d{x changed to {g%d{x.\r\n", insert_loc+1, pReset->bestTier);
            }

            if (pReset->command == 'E') {
                if (pReset->rand != RAND_NONE && pReset->arg3 == wear_wield) {
                    pReset->arg3 = wear_sheath;
                    stc("Changed wearlocation from 'wield' to 'sheath'.\r\n", ch);
                } else if (pReset->rand == RAND_NONE && pReset->arg3 == wear_sheath) {
                    pReset->arg3 = wear_wield;
                    stc("Changed wearlocation from 'sheath' to 'wield'.\r\n", ch);
                }
            }

            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            if (str_cmp(arg4, "quiet"))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

        if (arg_oneof(arg2, "tier", "тиер")) {
            int insert_loc = find_reset(pRoom, arg1);
            if (insert_loc < 0) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            int tier = atoi(arg3);
            if (tier < BEST_TIER || tier > WORST_TIER) {
                ptc(ch, "Tier must be a number from %d to %d.\r\n", BEST_TIER, WORST_TIER);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            pReset->bestTier = tier;
            ptc(ch, "Best tier for reset {W%d{x set to {g%d{x.\r\n", insert_loc+1, tier);
            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            if (str_cmp(arg4, "quiet"))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

        if (arg_oneof(arg2, "wearloc", "слот")) {
            int insert_loc = find_reset(pRoom, arg1);
            if (insert_loc < 0) {
                stc("Reset with this number not found.\r\n", ch);
                return;
            }

            pReset = pRoom->resets.at(insert_loc);
            if (pReset->command != 'E' && pReset->command != 'G') {
                ptc(ch, "Reset has to be a mob equipment or inventory reset.\r\n");
                return;
            }

            Wearlocation *wearloc = wearlocationManager->findExisting(arg3);
            if (!wearloc) {
                ptc(ch, "Wearloc slot '%s' not found, see {y{hcolchelp wearloc{x for a full list.\r\n", argument);
                return;
            }

            pReset->arg3 = wearloc->getIndex();
            ptc(ch, "Wearloc for %d is now %s.\r\n", insert_loc+1, wearloc->getName().c_str());

            if (pReset->arg3 == wear_none) {                
                if (pReset->command == 'E') {
                    pReset->command = 'G';
                    ptc(ch, "Also changing reset type from 'equip' to 'inventory'.\r\n");
                }
            } else if (pReset->command == 'G') {
                pReset->command = 'E';
                ptc(ch, "Also changing reset type from 'inventory' to 'equip'.\r\n");
            }

            SET_BIT(pRoom->areaIndex->area_flag, AREA_CHANGED);
            if (str_cmp(arg4, "quiet"))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

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
                    pReset->vnums.push_back(pReset->arg1);
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
                        stc("Resets: 'olchelp wearloc'\n\r", ch);
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
    stc("        RESET <number> RAND <normal|rand_stat|rand_all>\n\r", ch);
    stc("        RESET <number> TIER <1..5>\n\r", ch);
    stc("        RESET <number> WEAR <slot>\n\r", ch);
    stc("        RESET SEARCH <vnum>\n\r", ch);
    stc("        RESET SEARCH <vnum> normal|rand_stat|rand_all \n\r", ch);
    stc("        RESET SEARCH rand_stat|rand_all \n\r", ch);
}

// randomize <vnum> <rand_flag> [best_tier]
CMD(randomize, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Weapon generator support command.")
{
    DLString args = argument;
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args.getOneArgument();
    DLString arg3 = args.getOneArgument();

    if (arg1.empty() || arg2.empty()) {
        ch->println("Usage: randomize <vnum> <rand_flag> [best_tier]");    
        return;
    }

    Integer vnum;
    if (!Integer::tryParse(vnum, arg1)) {
        ch->println("Vnum is not a number.");
        return;
    }

    OBJ_INDEX_DATA *pObjIndex = get_obj_index(vnum);
    if (!pObjIndex) {
        ch->printf("Item with vnum %d doesn't exist.\r\n", vnum.getValue());
        return;
    }

    if (pObjIndex->item_type != ITEM_WEAPON) {
        ch->printf("Item %s [%d] is not a weapon.\r\n", 
                    russian_case(pObjIndex->short_descr, '1').c_str(), pObjIndex->vnum);
        return;
    }

    bool success = false;
    for (auto &r : roomIndexMap) {
        RoomIndexData *pRoom = r.second;
        int cnt = 0;

        for (auto &pReset : pRoom->resets) {
            cnt++;
            switch (pReset->command) {
            case 'G':
            case 'E':
            case 'O':
            case 'P':
                if (pReset->arg1 == pObjIndex->vnum) {
                    ch->printf("{CChanging %c reset in room [%d] %s:{x\r\n", pReset->command, pRoom->vnum, pRoom->name);

                    int old_value = pReset->rand;
                    DLString rargs = DLString(pRoom->vnum) + " resets " + DLString(cnt) + " rand " + arg2 + " quiet";
                    interpret_raw(ch, "at", rargs.c_str());
                    success |= (old_value != pReset->rand);

                    if (!arg3.empty()) {
                        int old_value = pReset->bestTier;
                        rargs = DLString(pRoom->vnum) + " resets " + DLString(cnt) + " tier " + arg3 + " quiet";
                        interpret_raw(ch, "at", rargs.c_str());
                        success |= (old_value != pReset->bestTier);
                    }
                }
                break;
            }
        }
    }

    if (success) {
        Object *obj, *obj_next;
        ostringstream buf;

        ch->println("{CReviewing all item locations:{x");
        
        for (obj = object_list; obj; obj = obj_next) {
            obj_next = obj->next;
            if (obj->pIndexData != pObjIndex)
                continue;

            Room *room = obj->getRoom();

            if (obj->reset_mob > 0 && obj->carried_by && obj->carried_by->getID() == obj->reset_mob) {
                buf << "...destroyed at mob [" << obj->carried_by->getNPC()->pIndexData->vnum << "] "
                    << "in room [" << room->vnum << "] " << room->getName() << endl;
            }
            else if (obj->reset_room > 0 && obj->in_room && obj->in_room->vnum == obj->reset_room) {
                buf << "...destroyed in room [" << room->vnum << "] " << room->getName() << endl;
            }
            else if (obj->reset_obj > 9 && obj->in_obj && obj->in_obj->getID() == obj->reset_obj) {
                buf << "...destroyed inside obj [" << obj->in_obj->pIndexData->vnum << "] in room "
                    << room->vnum << "] " << room->getName() << endl;
            } else {
                buf << "Cannot destroy item " << obj->getID() << " not in reset location, room " << obj->getRoom()->vnum << endl;
                continue;
            }

            extract_obj(obj);
            reset_room(room, FRESET_ALWAYS);
        }

        page_to_char(buf.str().c_str(), ch);
    }
}
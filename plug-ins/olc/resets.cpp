
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "wearlocation.h"
#include "loadsave.h"
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
#include "wearloc_utils.h"
#include "merc.h"
#include "resetops.h"

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

static DLString show_reset_flags(int iReset, RESET_DATA *pReset, RoomIndexData *pRoom)
{
    // Built by hand like the wear menu: the 'at <room>' id already names the room,
    // and it must stay a plain command outside any editor.
    DLString id = "at " + DLString(pRoom->vnum) + " reset " + DLString(iReset) + " flag";
    StringList commands;
    for (int i = 0; i < reset_flags.size; i++)
        commands.push_back("$ " + DLString(reset_flags.fields[i].name));

    DLString label = pReset->flags.names();
    return web_menu(commands, id, label.empty() ? DLString("{Iwnone{Ix") : label);
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

    if (pReset->rand != RAND_NONE || item_is_random(pObjIndex)) {
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
            if (pObj && reset_loads_obj(pReset, vnum))
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
                line = "M[" + web_cmd(ch, "medit $1", "%5d") + "] %-24.24N1{x %2d-%2d {g%s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1, 
                        pMob->getShortDescr(LANG_DEFAULT),
                        pReset->arg2, 
                        pReset->arg4,
                        show_reset_flags(iReset+1, pReset, pRoom).c_str());
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
                line = "O[" + web_cmd(ch, "oedit $1", "%5d") +"] {%c%-24.24N1{x %s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1, 
                        get_item_colour(pObj, pReset),
                        pObj->getShortDescr(LANG_DEFAULT),
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
                line = "         [" + web_cmd(ch, "oedit $1", "%5d") + "] {%c%-24.24N1{x %2d-%2d %s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1,
                        get_item_colour(pObj, pReset),
                        pObj->getShortDescr(LANG_DEFAULT),
                        pReset->arg2,
                        pReset->arg4,
                        show_reset_rand(ch, iReset+1, pReset, pObj, pRoom).c_str());

                // A put picks one of several vnums at random: list the others.
                if (pReset->vnums.size( ) > 1) {
                    buf << "                 or:";
                    for (size_t v = 1; v < pReset->vnums.size( ); v++)
                        buf << " " << web_cmd(ch, "oedit $1", DLString(pReset->vnums[v]));
                    buf << "\n\r";
                }
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
                buf << fmt(0, "Give/Equip Object %d - No Previous Mobile\n\r", pReset->arg1);
                break;
            }

            if (matches) {
                found = true;
                line = "         [" + web_cmd(ch, "oedit $1", "%5d") +"] {%c%-24.24N1{x %s %s{x\n\r";
                buf << fmt(0, line.c_str(),
                        pReset->arg1,
                        get_item_colour(pObj, pReset),
                        pObj->getShortDescr(LANG_DEFAULT),
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
        result << "Resets for room [{C" << web_cmd(ch, "redit $1", pRoom->vnum) << "{x] {W" << pRoom->name.get(LANG_DEFAULT) << "{x:" << endl;
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

/** The reset under its 1-based number, or 0 with the reason told to ch. */
static RESET_DATA * find_reset(Character *ch, RoomIndexData *pRoom, int number)
{
    if (number < 1 || number > (int)pRoom->resets.size()) {
        stc("Reset with this number not found.\r\n", ch);
        return 0;
    }

    return pRoom->resets.at(number - 1);
}

static bool reset_added(Character *ch, RoomIndexData *pRoom, int after, RESET_DATA *pReset)
{
    add_reset(pRoom, pReset, after);
    pRoom->areaIndex->changed = true;
    stc("Reset added.\n\r", ch);
    return true;
}

bool reset_add_mob(Character *ch, RoomIndexData *pRoom, int after, int vnum, int maxArea, int maxRoom)
{
    // A negative vnum reaching the index lookups crashes the server.
    if (vnum <= 0 || get_mob_index(vnum) == NULL) {
        stc("Монстр не существует.\n\r", ch);
        return false;
    }

    RESET_DATA *pReset = new reset_data();
    pReset->command = 'M';
    pReset->arg1 = vnum;
    pReset->arg2 = maxArea;
    pReset->arg3 = pRoom->vnum;
    pReset->arg4 = maxRoom;
    return reset_added(ch, pRoom, after, pReset);
}

bool reset_add_obj_room(Character *ch, RoomIndexData *pRoom, int after, int vnum)
{
    if (vnum <= 0 || get_obj_index(vnum) == NULL) {
        stc("Предметов с таким номером не существует.\n\r", ch);
        return false;
    }

    RESET_DATA *pReset = new reset_data();
    pReset->command = 'O';
    pReset->arg1 = vnum;
    pReset->arg2 = 0;
    pReset->arg3 = pRoom->vnum;
    pReset->arg4 = 0;
    return reset_added(ch, pRoom, after, pReset);
}

bool reset_add_obj_inside(Character *ch, RoomIndexData *pRoom, int after, int vnum,
                          int container, int limit, int count)
{
    if (vnum <= 0 || get_obj_index(vnum) == NULL) {
        stc("Предметов с таким номером не существует.\n\r", ch);
        return false;
    }

    OBJ_INDEX_DATA *pContainer = container > 0 ? get_obj_index(container) : 0;
    if (!pContainer || pContainer->item_type != ITEM_CONTAINER) {
        stc("Предмет не является контейнером.\n\r", ch);
        return false;
    }

    RESET_DATA *pReset = new reset_data();
    pReset->command = 'P';
    pReset->arg1 = vnum;
    pReset->arg2 = limit;
    pReset->arg3 = container;
    pReset->arg4 = count;
    pReset->vnums.push_back(pReset->arg1);
    return reset_added(ch, pRoom, after, pReset);
}

bool reset_add_obj_wear(Character *ch, RoomIndexData *pRoom, int after, int vnum, const DLString &wearloc)
{
    Wearlocation *wl = wearlocationManager->findExisting(wearloc);
    if (!wl) {
        stc("Resets: 'olchelp wearloc'\n\r", ch);
        return false;
    }

    if (vnum <= 0 || get_obj_index(vnum) == NULL) {
        stc("Предметов с таким номером не существует.\n\r", ch);
        return false;
    }

    RESET_DATA *pReset = new reset_data();
    pReset->arg1 = vnum;
    pReset->arg3 = wl->getIndex( );
    pReset->command = pReset->arg3 == wear_none ? 'G' : 'E';
    return reset_added(ch, pRoom, after, pReset);
}

bool reset_delete(Character *ch, RoomIndexData *pRoom, int number)
{
    RESET_DATA *pReset = find_reset(ch, pRoom, number);
    if (!pReset)
        return false;

    pRoom->resets.erase(pRoom->resets.begin() + (number - 1));
    ddeallocate(pReset);
    pRoom->areaIndex->changed = true;
    stc("Reset deleted.\n\r", ch);
    return true;
}

bool reset_toggle_flags(Character *ch, RoomIndexData *pRoom, int number, const DLString &flagNames)
{
    RESET_DATA *pReset = find_reset(ch, pRoom, number);
    if (!pReset)
        return false;

    bitnumber_t flags = reset_flags.bitstring(flagNames, false);
    if (flags == NO_FLAG) {
        stc("Flags not found, see {y{hcolchelp reset_flags{x.\r\n", ch);
        return false;
    }

    pReset->flags.toggleBit(flags);

    if (pReset->flags.getValue() != 0)
        ptc(ch, "Flags for reset {W%d{x toggled to {g%s{x.\r\n", number, pReset->flags.names().c_str());
    else
        ptc(ch, "Flags for reset {W%d{x cleared.\r\n", number);

    pRoom->areaIndex->changed = true;
    return true;
}

bool reset_set_rand(Character *ch, RoomIndexData *pRoom, int number, const DLString &randName)
{
    RESET_DATA *pReset = find_reset(ch, pRoom, number);
    if (!pReset)
        return false;

    int rand = rand_table.value(randName, false);
    if (rand == NO_FLAG) {
        stc("Rand value no found, see {y{hcolchelp rand_table{x.\r\n", ch);
        return false;
    }

    pReset->rand.setValue(rand);
    ptc(ch, "Rand value for reset {W%d{x set to {g%s{x.\r\n", number, pReset->rand.name().c_str());

    if (pReset->bestTier <= 0) {
        pReset->bestTier = DEFAULT_TIER;
        ptc(ch, "Best tier for reset {W%d{x changed to {g%d{x.\r\n", number, pReset->bestTier);
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

    pRoom->areaIndex->changed = true;
    return true;
}

bool reset_set_tier(Character *ch, RoomIndexData *pRoom, int number, int tier)
{
    RESET_DATA *pReset = find_reset(ch, pRoom, number);
    if (!pReset)
        return false;

    if (tier < BEST_TIER || tier > WORST_TIER) {
        ptc(ch, "Tier must be a number from %d to %d.\r\n", BEST_TIER, WORST_TIER);
        return false;
    }

    pReset->bestTier = tier;
    ptc(ch, "Best tier for reset {W%d{x set to {g%d{x.\r\n", number, tier);
    pRoom->areaIndex->changed = true;
    return true;
}

bool reset_set_wearloc(Character *ch, RoomIndexData *pRoom, int number, const DLString &wearlocName)
{
    RESET_DATA *pReset = find_reset(ch, pRoom, number);
    if (!pReset)
        return false;

    if (pReset->command != 'E' && pReset->command != 'G') {
        ptc(ch, "Reset has to be a mob equipment or inventory reset.\r\n");
        return false;
    }

    Wearlocation *wearloc = wearlocationManager->findExisting(wearlocName);
    if (!wearloc) {
        ptc(ch, "Wearloc slot '%s' not found, see {y{hcolchelp wearloc{x for a full list.\r\n", wearlocName.c_str());
        return false;
    }

    pReset->arg3 = wearloc->getIndex();
    ptc(ch, "Wearloc for %d is now %s.\r\n", number, wearloc->getName().c_str());

    if (pReset->arg3 == wear_none) {
        if (pReset->command == 'E') {
            pReset->command = 'G';
            ptc(ch, "Also changing reset type from 'equip' to 'inventory'.\r\n");
        }
    } else if (pReset->command == 'G') {
        pReset->command = 'E';
        ptc(ch, "Also changing reset type from 'inventory' to 'equip'.\r\n");
    }

    pRoom->areaIndex->changed = true;
    return true;
}

bool reset_randomize_exits(Character *ch, RoomIndexData *pRoom)
{
    for (auto &reset: pRoom->resets) {
        if (reset->command == 'R') {
            stc("Exits in this room are already randomized.\n\r", ch);
            return false;
        }
    }

    RESET_DATA *pReset = new reset_data();
    pReset->command = 'R';
    pReset->arg1 = pRoom->vnum;
    // Shuffle all six exits: the 'R' reset swaps doors arg3..arg2-1.
    pReset->arg2 = DIR_SOMEWHERE;
    pReset->arg3 = 0;

    add_reset(pRoom, pReset, -1);
    pRoom->areaIndex->changed = true;
    stc("Room exits will be randomized.\n\r", ch);
    return true;
}

static int number_or(const char *arg, int dflt)
{
    return is_number(arg) ? atoi(arg) : dflt;
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
    RoomIndexData *pRoom = ch->in_room->pIndexData;

    argument = one_argument(argument, arg1);
    
    if (!OLCState::can_edit( ch, ch->in_room->vnum )) {
        stc("Resets: Invalid security for editing this area.\n\r", ch);
        return;
    }

    if (arg1[0] == '\0') {
        if (!pRoom->resets.empty()) {
            ostringstream buf;
            display_resets(buf, ch, pRoom, filter_t());
            ch->send_to(buf);
        } else {
            stc("No resets in this room.\n\r", ch);
        }

        return;
    }

    // Randomize exits.
    if (arg_is(arg1, "void")) {
        reset_randomize_exits(ch, pRoom);
        return;
    }

    // Syntax: search <vnum>, search rand_stat, search <vnum> rand_stat
    if (arg_is(arg1, "search")) {
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
        int number = atoi(arg1);

        if (arg_is_strict(arg2, "delete")) {
            reset_delete(ch, pRoom, number);
            return;
        }

        if (arg_is(arg2, "flag")) {
            if (reset_toggle_flags(ch, pRoom, number, argument))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

        argument = one_argument(argument, arg3);
        argument = one_argument(argument, arg4);

        if (arg_is(arg2, "rand")) {
            if (reset_set_rand(ch, pRoom, number, arg3))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

        if (arg_is(arg2, "tier")) {
            if (reset_set_tier(ch, pRoom, number, atoi(arg3)))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

        if (arg_is(arg2, "wearloc")) {
            if (reset_set_wearloc(ch, pRoom, number, arg3))
                __do_resets(ch, const_cast<char *>(""));
            return;
        }

        argument = one_argument(argument, arg5);
        argument = one_argument(argument, arg6);
        argument = one_argument(argument, arg7);

        if (arg_is_strict(arg2, "mob") && is_number(arg3)) {
            reset_add_mob(ch, pRoom, number, atoi(arg3), number_or(arg4, 1), number_or(arg5, 1));
            return;
        }

        if (arg_is_strict(arg2, "obj") && is_number(arg3) && arg4[0] != '\0') {
            if (!str_prefix(arg4, "inside"))
                reset_add_obj_inside(ch, pRoom, number, atoi(arg3), number_or(arg5, 0),
                                     number_or(arg6, 1), number_or(arg7, 1));
            else if (arg_is_strict(arg4, "room"))
                reset_add_obj_room(ch, pRoom, number, atoi(arg3));
            else
                reset_add_obj_wear(ch, pRoom, number, atoi(arg3), arg4);
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
    stc("        RESET VOID -- randomized room exits \n\r", ch);
}

/* $Id$
 *
 * ruffina, 2004
 */
#include <algorithm>
#include <string.h>

#include "grammar_entities_impl.h"
#include "dlfilestream.h"
#include "util/regexp.h"
#include <xmldocument.h>
#include "stringset.h"
#include "wrapperbase.h"
#include "string_utils.h"

#include <skill.h>
#include <spell.h>
#include "defaultspell.h"
#include "basicskill.h"
#include <skillmanager.h>
#include "skillcommand.h"
#include "skillgroup.h"
#include "feniamanager.h"
#include "areabehaviormanager.h"
#include <affect.h>
#include <object.h>
#include <pcharacter.h>
#include "pcharactermanager.h"
#include <npcharacter.h>
#include <commandmanager.h>
#include "profession.h"
#include "race.h"
#include "mobbody.h"
#include "clanreference.h"
#include "room.h"

#include "olc.h"
#include "morphology.h"
#include "xmlmultistring.h"
#include "body.h"
#include "olcflags.h"
#include "olcstate.h"
#include "security.h"
#include "areahelp.h"

#include "damageflags.h"
#include "commandflags.h"
#include "occupations.h"
#include "weather.h"
#include "update_areas.h"
#include "websocketrpc.h"
#include "interp.h"
#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "save.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "weapontier.h"
#include "mobextras.h"
#include "loadsave.h"
#include "vnum.h"

#include "comm.h"
#include "def.h"

void obj_update();
void char_update();

GSN(none);
CLAN(none);
GROUP(clan);
WEARLOC(sheath);

using namespace std;


int next_index_in_area( Character *ch, AreaIndexData *pArea, int ndx_type, const std::set<int> *skip )
{
    if (!pArea)
        return -1;

    for (int i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        if (i <= 0 || !OLCState::can_edit( ch, i ))
            continue;

        if (skip && skip->count( i ))
            continue;

        switch (ndx_type) {
        case OLC_NDX_ROOM:
            if (!get_room_index( i ))
                return i;
            break;
        case OLC_NDX_OBJ:
            if (!get_obj_index( i ))
                return i;
            break;
        case OLC_NDX_MOB:
            if (!get_mob_index( i ))
                return i;
            break;
        }
    }

    return -1;
}

int next_room( Character *ch, RoomIndexData *r )
{
    return r ? next_index_in_area( ch, r->areaIndex, OLC_NDX_ROOM ) : -1;
}

int next_obj_index( Character *ch, RoomIndexData *r )
{
    return r ? next_index_in_area( ch, r->areaIndex, OLC_NDX_OBJ ) : -1;
}

int next_mob_index( Character *ch, RoomIndexData *r )
{
    return r ? next_index_in_area( ch, r->areaIndex, OLC_NDX_MOB ) : -1;
}

int olc_create_vnum( Character *ch, AreaIndexData *nextArea, const char *argument, int ndx_type,
                     const std::set<int> *skip )
{
    int value;

    if (arg_is_strict(argument, "next")) {
        value = next_index_in_area( ch, nextArea, ndx_type, skip );
        if (value < 0) {
            ch->pecho("Все внумы в этой зоне уже заняты!");
            return -1;
        }
    } else {
        // A negative vnum reaching the index lookups crashes the server.
        if (!is_number(argument) || (value = atoi(argument)) <= 0) {
            stc("Syntax:  create <vnum>|next\n\r", ch);
            return -1;
        }
    }

    if (!OLCState::get_vnum_area(value)) {
        stc("OLC:  That vnum is not assigned an area.\n\r", ch);
        return -1;
    }

    if (!OLCState::can_edit( ch, value )) {
        stc("OLC:  Vnum in an area you cannot build in.\n\r", ch);
        return -1;
    }

    bool exists = (ndx_type == OLC_NDX_ROOM && get_room_index( value ))
                  || (ndx_type == OLC_NDX_OBJ && get_obj_index( value ))
                  || (ndx_type == OLC_NDX_MOB && get_mob_index( value ));
    if (exists) {
        stc("OLC:  That vnum already exists.\n\r", ch);
        return -1;
    }

    if (skip && skip->count( value )) {
        stc("OLC:  That vnum is already taken earlier in this batch.\n\r", ch);
        return -1;
    }

    return value;
}

const char *
get_skill_name( int sn, bool verbose )
{
    Skill *skill = SkillManager::getThis( )->find( sn );

    if (skill)
        return skill->getName( ).c_str( );
    else if (verbose)
        return "none";
    else
        return "";
}

void
ptc(Character *ch, const char *fmt, ...)
{
    va_list av;

    va_start(av, fmt);

    DLString rc = vfmt(ch, fmt, av);
    stc(rc.c_str( ), ch);

    va_end(av);
}

void
ptc(Character *ch, const MultiMessage &fmt, ...)
{
    va_list av;

    va_start(av, fmt);

    DLString rc = vfmt(ch, fmt.getMessage(ch).c_str( ), av);
    stc(rc.c_str( ), ch);

    va_end(av);
}


/** Get next available help ID to use. Can potentially result in duplicates if a plugin 
    containing some helps is unloaded when this function is being called.  */
int help_next_free_id()
{
    list<int> ids;
    for (auto &a: helpManager->getArticles())
        ids.push_back(a->getID());

    ids.sort();

    int latest = 1;

    for (auto &id: ids) {
        if (id <= 0)
            continue;
            
        if (id - latest > 1)
            return latest + 1;

        latest = id;
    }

    return helpManager->getLastID() + 1;
}


struct editor_table_entry {
    const char *arg, *cmd;
} editor_table[] = {
    {"area",   "aedit"},
    {"room",   "redit"},
    {"object", "oedit"},
    {"mobile", "medit"},
    {"help",   "hedit"},
    {NULL, 0,}
};

CMD(edit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online editor.")
{
    char command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument(argument, command);

    if (command[0] == '\0') {
//        do_help(ch, "olc");
        return;
    }

    for (cmd = 0; editor_table[cmd].arg != NULL; cmd++) {
        if (!str_prefix(command, editor_table[cmd].arg)) {
            interpret_raw(ch, editor_table[cmd].cmd, "%s", argument);
            return;
        }
    }

//    do_help(ch, "olc");
}

static bool area_cmp_filename(const AreaIndexData *a, const AreaIndexData *b)
{
    return a->area_file->file_name < b->area_file->file_name;
}

static bool area_cmp_vnum(const AreaIndexData *a, const AreaIndexData *b)
{
    return a->min_vnum < b->min_vnum;
}

static bool area_cmp_name(const AreaIndexData *a, const AreaIndexData *b) 
{
    DLString name1 = a->getName().colourStrip();
    DLString name2 = b->getName().colourStrip();
    return name1.compareRussian(name2) < 0;
}

CMD(alist, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "List areas.")
{
    vector<AreaIndexData *> areas;
    DLString args(argument);
    DLString arg = args.getOneArgument();
    DLString flagName;

    for(auto &pArea: areaIndexes) {
        areas.push_back(pArea);
    }

    if (arg_is(arg, "vnum")) 
        sort(areas.begin(), areas.end(), area_cmp_vnum);
    else if (arg_is(arg, "name"))
        sort(areas.begin(), areas.end(), area_cmp_name);
    else if (arg_is(arg, "file"))
        sort(areas.begin(), areas.end(), area_cmp_filename);
    else if (arg_is(arg, "flag") && !args.empty())
        flagName = args;
    else if (!arg.empty()) {
        ch->pecho("Формат:\r\nalist - список всех арий\r\nalist vnum|name|file - список арий, отсортированный по критерию");
        ch->pecho("alist flag <name> - список всех арий с флагом name");
        return;
    }

    const DLString lineFormat = 
            "[" + web_cmd(ch, "aedit $1", "%3d") 
            + "] {%s%-29s {%s(%5u-%5u) %17s %s{w\n\r";

    ptc(ch, "[%3s] %-29s   (%5s-%5s) %-17s %s\n\r",
      "Num", "Area Name", "lvnum", "uvnum", "Filename", "Help");

    for (auto &pArea: areas) {
        if (!flagName.empty()) {
            DLString areaflags = area_flags.names(pArea->area_flag);
            if (areaflags.find(flagName) == DLString::npos)
                continue;
        }
            
        DLString hedit = "";
        AreaHelp *ahelp = area_selfhelp(pArea);
        if (ahelp && ahelp->getID() > 0) {
            DLString id(ahelp->getID());
            // Mark zones without meaningful help articles with red asterix.
            DLString color = help_is_empty(*ahelp) ? " {R*{x": "";
            hedit = web_cmd(ch, "hedit " + id, "hedit " + id) + color;
        }

        // System areas are shown in grey colors.           
        const char *colorAreaName = IS_SET(pArea->area_flag, AREA_SYSTEM) ? "D" : "W";
        const char *colorAreaVnums = IS_SET(pArea->area_flag, AREA_SYSTEM) ? "D" : "w";
        DLString areaName = pArea->getName().colourStrip();

        ch->send_to(
            fmt(0, lineFormat.c_str(), 
                pArea->vnum, 
                colorAreaName,
                String::truncate(areaName, 29).c_str(),
                colorAreaVnums,
                pArea->min_vnum, pArea->max_vnum,
                pArea->area_file->file_name.c_str(),
                hedit.c_str()));
    }
}

/*
 * abc body (mob reform, plan §3.6 item 6, §5.6): replaces abc size/part/aff/form.
 *   abc body                      prototypes with authored body diffs
 *   abc body dels                 unreviewed dels on aff/det/imm/res/vuln, [keep]/[drop]
 *   abc body keep|drop <vnum> <set>
 *   abc body instances            live mobs whose body differs from their prototype's
 */
static const char * const abc_set_names[MOBSET_MAX] = {
    "act", "off", "aff", "det", "imm", "res", "vuln", "form", "parts"
};

static const FlagTable *abc_set_table(int s)
{
    static const FlagTable *tables[MOBSET_MAX] = {
        &act_flags, &off_flags, &affect_flags, &detect_flags, &imm_flags, &res_flags, &vuln_flags,
        &form_flags, &part_flags
    };
    return tables[s];
}

static bool abc_set_gated(int s)
{
    return s == MOBSET_AFF || s == MOBSET_DET || s == MOBSET_IMM || s == MOBSET_RES || s == MOBSET_VULN;
}

/* Body::BitSet index of a gated MOBSET_ set, for mob_index_data::reviewed. */
static int abc_reviewed_bit(int s)
{
    switch (s) {
    case MOBSET_AFF:  return Body::BS_AFF;
    case MOBSET_DET:  return Body::BS_DET;
    case MOBSET_IMM:  return Body::BS_IMM;
    case MOBSET_RES:  return Body::BS_RES;
    case MOBSET_VULN: return Body::BS_VULN;
    }
    return -1;
}

static void abc_body(Character *ch, DLString &args)
{
    const int maxlines = 40;
    DLString mode = args.getOneArgument();
    ostringstream buf;
    int cnt = 0;

    if (mode == "keep" || mode == "drop") {
        DLString vnumArg = args.getOneArgument();
        DLString setArg = args.getOneArgument();
        Integer vnum;
        if (!Integer::tryParse(vnum, vnumArg))
            return;
        MOB_INDEX_DATA *pMob = get_mob_index(vnum);
        if (!pMob)
            return;

        for (int s = 0; s < MOBSET_MAX; s++) {
            if (setArg != abc_set_names[s] || !abc_set_gated(s))
                continue;

            bitstring_t before = (unsigned int)pMob->bodyBits(s);
            if (mode == "keep")
                pMob->reviewed |= 1 << abc_reviewed_bit(s);
            else
                pMob->bodyDel[s] = 0;

            mob_index_rebuild(pMob);
            pMob->area->changed = true;
            ch->pecho("Mob %d: %s dels %s.", vnum.getValue(), setArg.c_str(),
                      mode == "keep" ? "now apply" : "dropped");
            // Reviewed imm/res/vuln also switches the authored adds on.
            bitstring_t alive = pMob->aliveAdds(s, before);
            if (mode == "keep" && alive)
                ch->pecho("Mob %d: %s adds now apply too: [%s].", vnum.getValue(), setArg.c_str(),
                          MobBody::bitSetTable(abc_reviewed_bit(s))->names(alive).c_str());
            DLString again = "body dels";
            abc_body(ch, again);
            return;
        }
        return;
    }

    if (mode == "instances") {
        buf << fmt(0, "%7s %-18s %s", "VNUM", "NAME", "BODY DIFF vs PROTOTYPE") << endl;
        for (Character *wch = char_list; wch; wch = wch->next) {
            NPCharacter *mob = wch->getNPC();
            if (!mob)
                continue;

            ostringstream diff;
            for (int s = 0; s < MOBSET_MAX; s++) {
                bitstring_t produced = (unsigned int)mob->pIndexData->bodyBits(s);
                bitstring_t base = mob->baseBits[s];
                if (s == MOBSET_ACT)
                    produced |= ACT_IS_NPC;
                if (s == MOBSET_AFF)
                    produced &= ~(bitstring_t)AFF_FROM_AFFECTS;
                bitstring_t add = base & ~produced, del = produced & ~base;
                if (add)
                    diff << " " << abc_set_names[s] << " +{G" << abc_set_table(s)->names(add) << "{x";
                if (del)
                    diff << " " << abc_set_names[s] << " -{r" << abc_set_table(s)->names(del) << "{x";
            }
            if (diff.str().empty())
                continue;

            if (cnt < maxlines)
                buf << fmt(0, "[%5d] %-18.18N1", mob->pIndexData->vnum, mob->getNameP('1').c_str())
                    << diff.str() << endl;
            cnt++;
        }
        buf << "Found " << cnt << " mobs." << endl;
        page_to_char(buf.str().c_str(), ch);
        return;
    }

    bool dels = (mode == "dels");

    for (int i = 0; i < MAX_KEY_HASH; i++)
    for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next) {
        ostringstream line;
        DLString vnum = pMob->vnum;

        for (int s = 0; s < MOBSET_MAX; s++) {
            if (dels) {
                if (!abc_set_gated(s) || !pMob->bodyDel[s]
                        || (pMob->reviewed & (1 << abc_reviewed_bit(s))))
                    continue;
                line << " " << abc_set_names[s] << " -{r" << abc_set_table(s)->names(pMob->bodyDel[s]) << "{x "
                     << "[" << web_cmd(ch, "abc body keep " + vnum + " " + abc_set_names[s], "keep") << "]"
                     << "[" << web_cmd(ch, "abc body drop " + vnum + " " + abc_set_names[s], "drop") << "]";
                continue;
            }
            bitstring_t add = pMob->bodyAdd[s], del = pMob->bodyDel[s];
            if (s == MOBSET_ACT)
                add &= ~(bitstring_t)ACT_IS_NPC;
            if (add)
                line << " " << abc_set_names[s] << " +{G" << abc_set_table(s)->names(add) << "{x";
            if (del)
                line << " " << abc_set_names[s] << " -{r" << abc_set_table(s)->names(del) << "{x";
        }
        if (!dels && pMob->size != NO_FLAG && pMob->size != raceManager->find(pMob->race)->getSize())
            line << " size {Y" << size_table.name(pMob->size) << "{x";

        if (line.str().empty())
            continue;

        if (cnt < maxlines) {
            DLString head = "[" + web_cmd(ch, "medit $1", "%5d") + "] %-18.18N1 {g"
                            + web_cmd(ch, "raceedit $1", "%-10.10s") + "{x";
            buf << fmt(0, head.c_str(), pMob->vnum, pMob->getShortDescr(LANG_DEFAULT), pMob->race.c_str())
                << line.str() << endl;
        }
        cnt++;
    }

    buf << "Found " << cnt << " mobs." << endl;
    page_to_char(buf.str().c_str(), ch);
}

CMD(abc, 50, "", POS_DEAD, 106, LOG_ALWAYS, "")
{
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (arg == "ed") {
        int cnt = 0;
        ostringstream buf;

        for (auto &r: roomIndexMap) {
            int vnum = r.first;
            RoomIndexData *pRoom = r.second;
            DLString rdesc = pRoom->description[LANG_DEFAULT];
            if (rdesc.empty())
                rdesc = pRoom->description[EN];

            rdesc = rdesc.toLower().colourStrip();

            StringList descrs;

            for (auto &ed: pRoom->extraDescriptions) {
                DLString desc = String::stripEOL(ed->description.get(LANG_DEFAULT));
                descrs.push_back(desc.toLower());
            }

            for (int door = 0; door < DIR_SOMEWHERE; door++) {
                EXIT_DATA *pExit = pRoom->exit[door];

                if (pExit && !pExit->description.get(LANG_DEFAULT).empty())
                    descrs.push_back(pExit->description.get(LANG_DEFAULT));
            }

            for (auto &eexit: pRoom->extra_exits) {
                const DLString &ee_rdesc = eexit->room_description.get(LANG_DEFAULT);
                const DLString &ee_desc = eexit->description.get(LANG_DEFAULT);
                if (!ee_rdesc.empty())
                    descrs.push_back(ee_rdesc);
                if (!ee_desc.empty())
                    descrs.push_back(ee_desc);
            }
                
            for (auto &ed: pRoom->extraDescriptions) {
                bool foundRoom = false;
                bool foundOther = false;

                if (ed->keyword.strPrefix("owner"))
                    continue;

                XMLMultiString mlKeyword;
                mlKeyword.fromMixedString(ed->keyword);

                for (auto &kw: mlKeyword[RU].split(" ")) {
                    DLString k1 = kw.toLower();
                    DLString k2 = kw.substr(0, kw.size() - 2);

                    if (String::contains(rdesc, k1) || String::contains(rdesc, k2))
                        foundRoom = true;

                    for (auto &desc: descrs)
                        if (String::contains(desc, k1) || String::contains(desc, k2))
                            foundOther = true;
                }

                if (!foundRoom && !foundOther) {
                    buf << fmt(0, "[{W%6d{x] [{y%-13s{x] [{g%s{x] not found\n", 
                        vnum, 
                        pRoom->areaIndex->area_file->file_name.c_str(),
                        ed->keyword.c_str());
                    cnt++;
                }
            }

        }

        buf << "{GTotal: " << cnt << "{x" << endl;
        page_to_char(buf.str().c_str(), ch);

        return;
    }

    if (arg == "eexit") {
        ostringstream abuf, cbuf, mbuf;
        abuf << endl << "Экстравыходы везде:" << endl;
        mbuf << endl << "Экстравыходы в особняках и пригородах:" << endl;
        cbuf << endl << "Экстравыходы в кланах:" << endl;

        const DLString lineFormat = "[" + web_cmd(ch, "goto $1", "%5d") + "] [%15s] [{Y%30s{x] [{G%30s{x]";
        for (auto &room: roomInstances) {
            ostringstream *buf;
            if (IS_SET(room->room_flags, ROOM_MANSION) 
                    || DLString("ht").strPrefix(room->areaIndex()->area_file->file_name))
                buf = &mbuf;
            else if (room->pIndexData->clan != clan_none)
                buf = &cbuf;
            else
                buf = &abuf;
            for (auto &eexit: room->extra_exits) {
                (*buf) << fmt(0, lineFormat.c_str(), 
                                room->vnum, 
                                room->areaIndex()->area_file->file_name.c_str(),
                                eexit->short_desc_from[RU].c_str(),                           
                                eexit->short_desc_to[RU].c_str()) << endl;
            }
        }
        
        page_to_char( mbuf.str( ).c_str( ), ch );
        page_to_char( cbuf.str( ).c_str( ), ch );
        page_to_char( abuf.str( ).c_str( ), ch );

        return;
    }

    if (!ch->isCoder( ))
        return;


    if (arg == "maxhelp") {
        ch->pecho("Max help ID is %d, next free id is %d.", 
                    helpManager->getLastID(), help_next_free_id());
        return;
    }

    if (arg == "readroom") {
        Integer vnum;
        Room *room;

        if (args.empty() || !Integer::tryParse(vnum, args)) {
            ch->pecho("abc readroom <vnum>");
            return;
        }

        room = get_room_instance(vnum);
        if (!room) {
            ch->pecho("Room vnum [%d] not found.", vnum.getValue());
            return;
        }
        
        ch->pecho("Loading room objects for '%s' [%d], check logs for details.", 
                    room->getName(), room->vnum);
        load_room_objects(room, const_cast<char *>("/tmp"), false);
        return;
    }

    if (arg == "obj_update") {
        obj_update();
        ch->pecho("Forced obj update.");
        return;
    }

    if (arg == "char_update") {
        char_update();
        ch->pecho("Forced char update.");
        return;
    }

    if (arg == "weather_init") {
        weather_init();
        ch->pecho("Forced weather initialization.");
        return;
    }

    if (arg == "badresets") {
        ostringstream buf;

        buf << "List of all resets with invalid wearlocations: " << endl;
        for (auto &r: roomIndexMap) {
            MOB_INDEX_DATA *pMob = 0;

            for (auto &pReset: r.second->resets) {
                if (pReset->command == 'O' || pReset->command == 'R') {
                    pMob = 0;
                    continue;
                }

                if (pReset->command == 'M') {
                    pMob = get_mob_index(pReset->arg1);
                    continue;
                }

                if (pReset->command == 'E') {
                    if (!pMob) {
                        buf << "Bad mob reset " << pReset->arg1 << " in room " << r.first << endl;
                        continue;
                    }

                    Wearlocation *wloc = wearlocationManager->find(pReset->arg3);
                    if (!wloc) {
                        buf << "Bad wearloc for mob " << pMob->vnum << " in room " << r.first << endl;
                        continue;
                    }

                    if (wloc->getIndex() == wear_sheath)
                        continue;

                    Race *race = raceManager->find(pMob->race);
                    if (!race->getWearloc().isSet(*wloc)) {
                        buf << "[" << r.first << "] mob [" << pMob->vnum << "] race '" 
                            << race->getName() <<"' equipped at " << wloc->getName() << endl;
                    }
                }
            }
        }

        page_to_char(buf.str().c_str(), ch);
        return;
    }

    if (arg == "body") {
        abc_body(ch, args);
        return;
    }
}


bool olc_same_xml( const XMLNode::Pointer &a, const XMLNode::Pointer &b )
{
    if (!a || !b)
        return !a && !b;

    return a->equal(**b);
}

/*--------------------------------------------------------------------------
 * Case forms of prototype short descriptions
 *-------------------------------------------------------------------------*/
DLString olc_cases_gender_mob( int sex, bool plural )
{
    if (plural)
        return "plur";
    if (sex == SEX_FEMALE)
        return "femn";
    if (sex == SEX_NEUTRAL)
        return "neut";
    return "masc";
}

DLString olc_cases_gender_obj( int multiGender )
{
    switch (multiGender) {
    case Grammar::MultiGender::FEMININE: return "femn";
    case Grammar::MultiGender::NEUTER:   return "neut";
    case Grammar::MultiGender::PLURAL:   return "plur";
    default:                             return "masc";
    }
}

static bool olc_cases_fill_lang( DLString &value, lang_t lang, const DLString &gender, bool anim,
                                 bool force, DLString &report )
{
    DLString label = lang == LANG_RU ? "RU" : "UA";
    bool hasPad = value.find( '|' ) != DLString::npos;

    if (!String::hasCyrillic( value ) || (hasPad && !force))
        return false;

    // A colour code would land inside a case cell, or split a word in two.
    if (value.find( '{' ) != DLString::npos) {
        report << label << " short has colour codes: fill its cases by hand.\r\n";
        return false;
    }

    DLString nominative = hasPad ? value.ruscase( '1' ) : value;
    // The sidecar reads the animacy after the comma and declines the phrase
    // as a prototype name: head noun, agreeing adjectives, fixed tail.
    DLString request = gender + (anim ? ",anim" : ",inan");
    DLString pad = lang == LANG_RU
                    ? Morphology::declineRu( nominative, "-", request )
                    : Morphology::declineUa( nominative, "-", request );

    // decline_sidecar's fallback when the sidecar is down or still loading.
    if (pad == nominative + "|||||") {
        report << label << " cases not filled: the morphology service did not answer, try {Wcases auto{x later.\r\n";
        return false;
    }

    if (pad.find( '|' ) == DLString::npos) {
        if (force)
            report << label << " short does not decline, left as typed.\r\n";
        return false;
    }

    if (pad.ruscase( '1' ) != nominative) {
        report << label << " cases not filled: the answer would change the name to '" << pad.ruscase( '1' ) << "'.\r\n";
        return false;
    }

    if (pad == value) {
        if (force)
            report << label << " cases already match: " << pad << "\r\n";
        return false;
    }

    value = pad;
    report << label << " cases filled: " << pad << "\r\n";
    if (!force)
        report << "  To keep it undeclined, type it with a trailing |||||.\r\n";
    return true;
}

static bool olc_cases_fill( XMLMultiString &shortDescr, const XMLMultiString *before,
                            const DLString &gender, bool anim, bool force,
                            bool ru, bool ua, DLString &report )
{
    bool changed = false;

    for (auto &l: shortDescr) {
        if (!((l.first == LANG_RU && ru) || (l.first == LANG_UA && ua)))
            continue;

        if (before && before->get( l.first ) == l.second)
            continue;

        if (olc_cases_fill_lang( l.second, l.first, gender, anim, force, report ))
            changed = true;
    }

    return changed;
}

bool olc_cases_autofill( XMLMultiString &shortDescr, const XMLMultiString *before,
                         const DLString &gender, bool anim, DLString &report )
{
    return olc_cases_fill( shortDescr, before, gender, anim, false, true, true, report );
}

void olc_cases_show( PCharacter *ch, const XMLMultiString &shortDescr )
{
    static const char *names[] = { "nom", "gen", "dat", "acc", "ins", "loc" };
    static const lang_t langs[] = { LANG_RU, LANG_UA };

    for (auto lang: langs) {
        const DLString &value = shortDescr.get( lang );
        const char *label = lang == LANG_RU ? "RU" : "UA";

        if (value.empty( ))
            continue;

        if (value.find( '|' ) == DLString::npos) {
            ptc( ch, "Short %s has no case forms, every case reads: %s\n\r", label, value.c_str( ) );
            continue;
        }

        ptc( ch, "Short %s cases:\n\r", label );
        for (int c = 0; c < 6; c++)
            ptc( ch, "  %s: %s\n\r", names[c], value.ruscase( (char)('1' + c) ).c_str( ) );
    }
}

bool olc_cases_command( PCharacter *ch, const char *argument, XMLMultiString &shortDescr,
                        const DLString &gender, bool anim )
{
    DLString args = argument;
    DLString word = args.getOneArgument( );
    DLString which = args.getOneArgument( );

    if (word.empty( )) {
        olc_cases_show( ch, shortDescr );
        return false;
    }

    bool ru = which.empty( ) || which == "ru";
    bool ua = which.empty( ) || which == "ua";

    if ((!word.strPrefix( "auto" ) && !word.strPrefix( "авто" )) || (!ru && !ua)) {
        ch->send_to( "Usage:\r\n"
                     "{Wcases{x - show the six cases of the RU and UA short\r\n"
                     "{Wcases auto{x [{Wru{x|{Wua{x] - decline the short again from its nominative\r\n" );
        return false;
    }

    DLString report;
    bool changed = olc_cases_fill( shortDescr, 0, gender, anim, true, ru, ua, report );

    if (report.empty( ))
        report = "Nothing to decline: no Cyrillic short.\r\n";

    ch->send_to( report );

    if (changed)
        olc_cases_show( ch, shortDescr );

    return changed;
}

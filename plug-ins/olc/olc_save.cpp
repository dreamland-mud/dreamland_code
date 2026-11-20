/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>
#include <fstream>

#include "jsoncpp/json/json.h"
#include "iconvmap.h"
#include "arg_utils.h"

#include "olc.h"
#include "onlinecreation.h"
#include "olcstate.h"
#include "xmlarea.h"
#include "dreamland.h"
#include "room.h"
#include "areautils.h"
#include "merc.h"
#include "def.h"

void
save_xmlarea_list( ) 
{
    XMLListBase<XMLString> lst;
    struct area_file *afile;
    
    for (afile = area_file_list; afile; afile = afile->next)
        lst.push_front(XMLString(afile->file_name));
    
    XMLNode::Pointer node(NEW);
    lst.toXML(node);
    node->setName("arealist");

    XMLDocument::Pointer doc(NEW);
    doc->appendChild(node);
    
    ofstream os(DLFile(dreamland->getAreaDir( ), dreamland->getAreaListFile( ), ".xml").getPath( ).c_str( ));
    doc->save(os);
}

bool save_xmlarea(struct area_file *af, Character *ch)
{
    try {
        XMLArea a;
        a.areadata.loaded = true;
        a.save(af);
    } catch (const ExceptionDBIO &ex) {
        if (ch)
            ch->pecho("{RERROR:{x: %s", ex.what());

        return false;
    }

    af->area->changed = false;
    return true;
}

int save_changed_areas(Character *ch, list<AreaIndexData *> &saved)
{
    int failed = 0;

    save_xmlarea_list();

    for(auto &pArea: areaIndexes) {
        /* Builder must be assigned this area. */
        if (!OLCState::can_edit(ch, pArea))
            continue;

        if (!pArea->changed)
            continue;

        if (save_xmlarea(pArea->area_file, ch))
            saved.push_back(pArea);
        else
            failed++;
    }

    return failed;
}

CMD(asave, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
    "Save areas.")
{
    char arg1[MAX_INPUT_LENGTH];
    AreaIndexData *pArea;
    int value;

    if (!ch) {
        save_xmlarea_list();
        struct area_file *afile;
        
        for (afile = area_file_list; afile; afile = afile->next)
            save_xmlarea(afile, ch);

        return;
    }

    strcpy(arg1, argument);
    if (arg1[0] == '\0') {
        stc("Syntax:\n\r", ch);
        stc("  asave <vnum>   - saves a particular area\n\r", ch);
        stc("  asave list     - saves the area.lst file\n\r", ch);
        stc("  asave area     - saves the area character currently in\n\r", ch);
        stc("  asave changed  - saves all changed zones\n\r", ch);
        stc("  asave world    - saves the world! (db dump)\n\r", ch);
        stc("\n\r", ch);
        return;
    }

    // Snarf the value (which need not be numeric)
    value = atoi(arg1);
    if (!(pArea = AreaUtils::getByVnum(value)) && is_number(arg1)) {
        stc("That area does not exist.\n\r", ch);
        return;
    }

    // Save area of given vnum.
    if (is_number(arg1)) {
        if (!OLCState::can_edit(ch, pArea)) {
            stc("You are not a builder for this area.\n\r", ch);
            return;
        }
        save_xmlarea_list();
        save_xmlarea(pArea->area_file, ch);
        return;
    }

    // Save the world, only authorized areas
    if (arg_is_strict(arg1, "world")) {
        save_xmlarea_list();
        struct area_file *afile;
        bool success = true;
        
        for (afile = area_file_list; afile; afile = afile->next) {
            pArea = afile->area;
            if (!OLCState::can_edit(ch, pArea))
                continue;

            if (!save_xmlarea(afile, ch))
                success = false;
        }

        if (success)
            stc("Все арии сохранены.\n\r", ch);
        else
            stc("Арии сохранены с ошибками!\n\r", ch);

        return;
    }

    // Save changed areas, only authorized areas
    if (arg_is_strict(arg1, "changed")) {
        list<AreaIndexData *> saved;

        save_changed_areas(ch, saved);

        stc("Saved zones:\n\r", ch);
        for (auto &pArea: saved)
            ptc(ch, "%24s - '%s'\n\r", pArea->getName().c_str(), pArea->area_file->file_name.c_str());

        if (saved.empty())
            stc("None\r\n", ch);
        return;
    }

    // Save the area.lst file
    if (arg_is_strict(arg1, "list")) {
        save_xmlarea_list();
        return;
    }

    // Save area being edited, if authorized
    if (arg_is_strict(arg1, "area")) {
        pArea = ch->in_room->areaIndex();

        if (!pArea || !OLCState::can_edit(ch, pArea)) {
            stc("У вас нет прав изменять эту арию.\n\r", ch);
            return;
        }

        save_xmlarea_list();

        if (save_xmlarea(pArea->area_file, ch))
            stc("Area saved.\n\r", ch);

        return;
    }
    __do_asave(ch, const_cast<char *>(""));
}

/*
 * 'olc status': what the live-to-git pusher (scripts/olc-push.py) last
 * recorded. It runs from cron and keeps its state under the runtime base.
 */
static const char *OLC_PUSH_STATE = "var/run/olc-push.json";
static IconvMap utf2koi("utf-8", "koi8-u");

CMD(olc, 50, "", POS_DEAD, 103, LOG_NORMAL,
    "OLC sync status.")
{
    if (!ch->is_immortal()) {
        stc("Это не для тебя.\n\r", ch);
        return;
    }

    if (!arg_is_strict(argument, "status")) {
        stc("Syntax: olc status\n\r", ch);
        return;
    }

    std::ifstream in(DLFile(dreamland->getBasePath(), OLC_PUSH_STATE).getPath().c_str());
    if (!in) {
        stc("OLC push: pusher not installed.\n\r", ch);
        return;
    }

    Json::Value st;
    Json::Reader reader;
    if (!reader.parse(in, st) || !st.isObject()) {
        ptc(ch, "OLC push: %s is unreadable.\n\r", OLC_PUSH_STATE);
        return;
    }

    DLString head = st["head"].isString() ? DLString(st["head"].asString()).substr(0, 10) : DLString("?");
    ptc(ch, "OLC push: live HEAD %s.\n\r", head.c_str());

    const Json::Value &held = st["held"];
    if (!held.isObject() || held.empty())
        stc("Held: none.\n\r", ch);
    else {
        ptc(ch, "Held, %d file(s):\n\r", (int)held.size());
        for (auto &f: held.getMemberNames())
            ptc(ch, "  %s: %s\n\r", utf2koi(f).c_str(),
                utf2koi(held[f].isString() ? held[f].asString() : held[f].toStyledString()).c_str());
    }

    const Json::Value &skips = st["skips"];
    if (!skips.isObject() || skips.empty())
        stc("Skipped: none.\n\r", ch);
    else {
        ptc(ch, "Skipped, %d file(s):\n\r", (int)skips.size());
        for (auto &f: skips.getMemberNames())
            ptc(ch, "  %s: %d run(s) in a row\n\r", utf2koi(f).c_str(),
                skips[f].isInt() ? skips[f].asInt() : 0);
    }
}

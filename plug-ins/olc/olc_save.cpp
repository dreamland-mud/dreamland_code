/* $Id$
 *
 * ruffina, 2004
 */

#include "olc.h"
#include "onlinecreation.h"
#include "olcstate.h"
#include "xmlarea.h"
#include "dreamland.h"
#include "room.h"
#include "merc.h"
#include "mercdb.h"
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

void
save_xmlarea(struct area_file *af)
{
    XMLArea a;
    a.areadata.loaded = true;
    a.save(af);
}

CMD(asave, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
    "Save areas.")
{
    char arg1[MAX_INPUT_LENGTH];
    AREA_DATA *pArea;
    int value;

    if (!ch) {
        save_xmlarea_list();
        struct area_file *afile;
        
        for (afile = area_file_list; afile; afile = afile->next) {
            if(afile->area) {
                if (afile->area && IS_SET(afile->area->area_flag, AREA_SAVELOCK)) {
                    ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
                    continue;
                }
                REMOVE_BIT(afile->area->area_flag, AREA_CHANGED);
            }
            save_xmlarea(afile);
        }
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
    if (!(pArea = get_area_data(value)) && is_number(arg1)) {
        stc("That area does not exist.\n\r", ch);
        return;
    }

    // Save area of given vnum.
    if (is_number(arg1)) {
        if (!OLCState::can_edit(ch, pArea)) {
            stc("You are not a builder for this area.\n\r", ch);
            return;
        }
        if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
            ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
            return;
        }
        save_xmlarea_list();
        save_xmlarea(pArea->area_file);
        return;
    }

    // Save the world, only authorized areas
    if (!str_cmp("world", arg1)) {
        save_xmlarea_list();
        struct area_file *afile;
        
        for (afile = area_file_list; afile; afile = afile->next) {
            pArea = afile->area;
            if(pArea) {
                if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
                    ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
                    continue;
                }
                if (!OLCState::can_edit(ch, pArea))
                    continue;

                REMOVE_BIT(pArea->area_flag, AREA_CHANGED);
            }

            save_xmlarea(afile);
        }
        stc("Все арии сохранены\n\r", ch);
        return;
    }

    // Save changed areas, only authorized areas
    if (!str_cmp("changed", arg1)) {
        char buf[MAX_INPUT_LENGTH];

        save_xmlarea_list();

        stc("Saved zones:\n\r", ch);
        sprintf(buf, "None.\n\r");

        for (pArea = area_first; pArea; pArea = pArea->next) {
            /* Builder must be assigned this area. */
            if (!OLCState::can_edit(ch, pArea))
                continue;

            if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
                ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
                continue;
            }
            /* Save changed areas. */
            if (IS_SET(pArea->area_flag, AREA_CHANGED)) {
                REMOVE_BIT(pArea->area_flag, AREA_CHANGED);
                save_xmlarea(pArea->area_file);
                ptc(ch, "%24s - '%s'\n\r", pArea->name, pArea->area_file->file_name);
            }
        }
        if (!str_cmp(buf, "None.\n\r"))
            stc(buf, ch);
        return;
    }

    // Save the area.lst file
    if (!str_cmp(arg1, "list")) {
        save_xmlarea_list();
        return;
    }

    // Save area being edited, if authorized
    if (!str_cmp(arg1, "area")) {
        pArea = ch->in_room->area;

        if (IS_SET(pArea->area_flag, AREA_SAVELOCK)) {
            ptc(ch, "Area %s was not saved! (Locked from saving)\n\r", ch);
            return;
        }
        if (!pArea || !OLCState::can_edit(ch, pArea)) {
            stc("У вас нет прав изменять эту арию.\n\r", ch);
            return;
        }

        save_xmlarea_list();
        REMOVE_BIT(pArea->area_flag, AREA_CHANGED);
        save_xmlarea(pArea->area_file);
        stc("Area saved.\n\r", ch);
        return;
    }
    __do_asave(ch, str_empty);
}

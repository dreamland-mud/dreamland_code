/* $Id$
 *
 * ruffina, 2004
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <config.h>

#include "char.h"
#include "grammar_entities_impl.h"
#include <skillmanager.h>
#include <character.h>
#include <pcharacter.h>
#include <commandmanager.h>
#include <object.h>
#include <affect.h>
#include "affectmanager.h"
#include "room.h"
#include "skillgroup.h"
#include "eventbus.h"

#include "itemevents.h"
#include "oedit.h"
#include "feniatriggers.h"
#include "ovalues.h"
#include "comm.h"
#include "merc.h"
#include "interp.h"
#include "weapontier.h"
#include "websocketrpc.h"
#include "act.h"
#include "mercdb.h"
#include "../anatolia/handler.h"

#include "olc.h"
#include "security.h"

#include "def.h"

#define EDIT_OBJ(x, y) y = &obj

OLC_STATE(OLCStateObject);

OLCStateObject::OLCStateObject( )
{
    /*fromXML will fill fields for us*/
}

OLCStateObject::OLCStateObject( OBJ_INDEX_DATA *original )
{
    obj.vnum         = original->vnum;
    obj.reset_num    = original->reset_num;
    obj.area         = original->area;

    if(original->behavior)
        obj.behavior         = XMLDocument::Pointer( NEW, **original->behavior );
    else
        obj.behavior         = 0;

    copyParameters( original );
    copyDescriptions( original );

}

OLCStateObject::OLCStateObject( int vnum )
{
    obj.vnum = vnum;
    obj.area = get_vnum_area(vnum);
}

OLCStateObject::~OLCStateObject( )
{
}

void OLCStateObject::copyParameters( OBJ_INDEX_DATA *original )
{
    for (auto &paf: original->affected)
        obj.affected.push_back(paf->clone());
    
    obj.item_type    = original->item_type;
    obj.extra_flags  = original->extra_flags;
    obj.wear_flags   = original->wear_flags;
    obj.level        = original->level;
    obj.condition    = original->condition;
    obj.weight       = original->weight;
    obj.cost         = original->cost;
    obj.limit        = original->limit;
    memcpy(obj.value, original->value, sizeof(obj.value));
}

void OLCStateObject::copyDescriptions( OBJ_INDEX_DATA *original )
{
    EXTRA_DESCR_DATA *ed, **my_ed = &obj.extra_descr;
    list<EXTRA_DESCR_DATA *> edlist;

    for(ed = original->extra_descr; ed; ed = ed->next)
        edlist.push_back(ed);
   
    while(!edlist.empty()) {
        ed = edlist.back();
        edlist.pop_back();
        *my_ed = new_extra_descr();
        (*my_ed)->keyword = str_dup(ed->keyword);
        (*my_ed)->description = str_dup(ed->description);
        my_ed = &(*my_ed)->next;
    }
    *my_ed = 0;

    obj.name         = str_dup(original->name);
    obj.short_descr  = str_dup(original->short_descr);
    obj.description  = str_dup(original->description);
    obj.smell        = original->smell;
    obj.sound        = original->sound;
    obj.material     = str_dup(original->material);
    obj.gram_gender  = original->gram_gender;

    obj.properties.clear( );
    for (Properties::const_iterator p = original->properties.begin( ); p != original->properties.end( ); p++)
        obj.properties.insert( *p );
}

void OLCStateObject::commit()
{
    Object *o;
    OBJ_INDEX_DATA *original;

    original = get_obj_index(obj.vnum);

    if(!original) {
        int iHash;
        
        original = new_obj_index();

        original->vnum = obj.vnum;
        original->area = obj.area;

        if (obj.vnum > top_vnum_obj)
            top_vnum_obj = obj.vnum;

        iHash = (int) obj.vnum % MAX_KEY_HASH;
        original->next = obj_index_hash[iHash];
        obj_index_hash[iHash] = original;
        top_obj_index++;
    }
    
    EXTRA_DESCR_DATA *ed, *ed_next;
    for(ed = original->extra_descr; ed; ed = ed_next) {
        ed_next = ed->next;
        free_extra_descr(ed);
    }
    original->extra_descr = obj.extra_descr;
    obj.extra_descr = 0;
    
    original->affected.deallocate();
    original->affected.assign(obj.affected.begin(), obj.affected.end());
    obj.affected.clear();
    
    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original) {
            /*object instance hase separate extra descr*/
            
            /*XXX - add/strip affects?*/
            
            if(o->item_type == original->item_type)
                o->item_type = obj.item_type;
            
            if(o->extra_flags == original->extra_flags)
                o->extra_flags = obj.extra_flags;

            if(o->wear_flags == original->wear_flags)
                o->wear_flags   = obj.wear_flags;

            if(o->level == original->level)
                o->level = obj.level;
            
            if(o->condition == original->condition)
                o->condition = obj.condition;
            
            if(o->weight == original->weight)
                o->weight = obj.weight;

            if(o->cost == original->cost)
                o->cost = obj.cost;
           
            for (int i = 0; i < 5; i++)
                if (o->getsValueFromProto(i) && o->valueByIndex(i) == original->value[i])
                    o->valueByIndex(i, obj.value[i]);
        }
    
    free_string(original->name);
    original->name         = obj.name;
    obj.name = 0;
    free_string(original->short_descr);
    original->short_descr  = obj.short_descr;
    obj.short_descr = 0;
    free_string(original->description);
    original->description  = obj.description;
    obj.description = 0;
    original->sound        = obj.sound;
    obj.sound.clear( );
    original->smell        = obj.smell;
    obj.smell.clear( );
    original->vnum         = obj.vnum;
    original->reset_num    = obj.reset_num;
    free_string(original->material);
    original->material     = obj.material;
    obj.material = 0;
    
    original->behavior     = obj.behavior;
    obj.behavior = 0; 
    original->item_type    = obj.item_type;
    original->extra_flags  = obj.extra_flags;
    original->wear_flags   = obj.wear_flags;
    original->level        = obj.level;
    original->condition    = obj.condition;
    original->gram_gender  = obj.gram_gender;
    original->weight       = obj.weight;
    original->cost         = obj.cost;
    memcpy(original->value, obj.value, sizeof(obj.value));
    original->limit        = obj.limit;

    original->properties.clear( );
    for (Properties::const_iterator p = obj.properties.begin( ); p != obj.properties.end( ); p++)
        original->properties.insert( *p );
    obj.properties.clear( );

    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original) {
            o->updateCachedNoun( );
            eventBus->publish(ItemEditedEvent(o));
        }
    
}

void OLCStateObject::statePrompt(Descriptor *d)
{
    d->send( "Editing obj> " );
}

OEDIT(show)
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    int cnt = 0;
    bool showWeb = !arg_oneof_strict(argument, "noweb");

    EDIT_OBJ(ch, pObj);

    sprintf(buf, "Name:        [%s] %s\n\rArea:        [%5d] %s\n\r",
              pObj->name,
              web_edit_button(showWeb, ch, "name", "web").c_str(),
              !pObj->area ? -1 : pObj->area->vnum,
              !pObj->area ? "No Area" : pObj->area->getName().c_str());
    stc(buf, ch);


    sprintf(buf, "Vnum:        [%7d]\n\r", pObj->vnum);
    stc(buf, ch);

    sprintf(buf, "Type:        [%s] {D(? item_table){x\n\r",
              item_table.name(pObj->item_type).c_str());
    stc(buf, ch);

    sprintf(buf, "Level:       [%5d]\n\r", pObj->level);
    stc(buf, ch);
    
    sprintf(buf, "Limit:       [%5d]\n\r", pObj->limit);
    stc(buf, ch);

    sprintf(buf, "Wear flags:  [%s] {D(? wear_flags){x\n\r",
              wear_flags.names(pObj->wear_flags).c_str());
    stc(buf, ch);

    sprintf(buf, "Extra flags: [%s] {D(? extra_flags){x\n\r",
              extra_flags.names(pObj->extra_flags).c_str());
    stc(buf, ch);

    ptc(ch, "Material:    [%s] {D(? material){x\n\r", pObj->material);

    ptc(ch, "Smell: [%s]\n\r", pObj->smell.c_str( ));

    ptc(ch, "Sound: [%s]\n\r", pObj->sound.c_str( ));

    if (!pObj->properties.empty( )) {
        ptc(ch, "Properties:\n\r");
        for (Properties::const_iterator p = pObj->properties.begin( ); p != pObj->properties.end( ); p++)
            ptc(ch, "{g%20s{x: %s\n\r", p->first.c_str( ), p->second.c_str( ));
    }

    ptc(ch, "Condition:   [%5d]\n\r", pObj->condition);

    ptc(ch, "Gender:      [%1s]\n\r", pObj->gram_gender.toString());

    ptc(ch, "Weight:      [%5d]\n\r", pObj->weight);
    
    if (!pObj->properties.count("random"))
        ptc(ch, "Cost:        [%5d]\n\r", pObj->cost);

    if (pObj->extra_descr) {
        EXTRA_DESCR_DATA *ed;

        stc("Extra desc: ", ch);

        for (ed = pObj->extra_descr; ed; ed = ed->next) {
            ptc(ch, "[%s] %s ", ed->keyword, web_edit_button(showWeb, ch, "ed web", ed->keyword).c_str());
        }

        stc(" {D(ed help){x\n\r", ch);
    }

    sprintf(buf, "Short desc:  %s %s\n\rLong desc: %s\n\r     %s\n\r",
              pObj->short_descr, web_edit_button(showWeb, ch, "short", "web").c_str(),
              web_edit_button(showWeb, ch, "long", "web").c_str(), pObj->description);
    stc(buf, ch);

    for (auto &paf: pObj->affected) {
        if (cnt == 0) {
            stc("Number  Location  Modifier  Table/Registry  Bits\n\r", ch);
            stc("------  --------  --------  --------------  ---------------------------\n\r", ch);
        }

        sprintf(buf, "[%4d]  %-8.8s  %8d", cnt,
                  paf->location.name().c_str(),
                  paf->modifier.getValue());
       
        const FlagTable *table = paf->bitvector.getTable();
        const GlobalRegistryBase *registry = paf->global.getRegistry();

        if ((table && paf->bitvector.getValue()) || registry) { 
            sprintf(buf+strlen(buf), "  %-14.14s  ",
                      registry ? registry->getRegistryName().c_str() : paf->bitvector.getTableName().c_str());

            if (registry) {
                sprintf(buf+strlen(buf), "%s", paf->global.toString().c_str());

                if (registry == liquidManager) {
                    strcat(buf, " {D(? liquid){x");
                } else if (registry == skillManager) {
                                        
                } else if (registry == skillGroupManager) {
                    strcat(buf, " {D(? practicer){x");
                } else if (registry == wearlocationManager) {
                    strcat(buf, " {D(? wearloc){x");
                } else {
                    strcat(buf, "<unknown registry>");
                }

            } else {
                strcat(buf, paf->bitvector.names().c_str());
                sprintf(buf+strlen(buf), " {D(? %s){x", paf->bitvector.getTableName().c_str());
            }
        }

        strcat(buf, "\n\r");
        stc(buf, ch);
        cnt++;
    }
    if (!pObj->affected.empty())
        stc("{D        ? apply_flags{x\r\n", ch);

    show_obj_values(ch, pObj);

    if (pObj->behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            pObj->behavior->save( ostr );
            ptc(ch, "Behavior:\r\n%s\r\n", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Behavior is BUGGY.\r\n");
        }
    }

    OBJ_INDEX_DATA *original = get_obj_index(obj.vnum);
    feniaTriggers->showTriggers(ch, original ? get_wrapper(original->wrapper) : 0, "obj");
    return false;
}

OEDIT(fenia)
{
    feniaTriggers->openEditor(ch, obj, argument);
    return false;
}

OEDIT(where)
{
    ptc(ch, "%s находится:\r\n", DLString( obj.short_descr ).ruscase('1').c_str( ));
    
    for (Object *o = object_list; o; o = o->next) {
        Character *wch;
        Room *room;

        if (o->pIndexData->vnum != obj.vnum)
            continue;
        
        room = o->getRoom( );
        wch = o->getCarrier( );

        if (wch)
            ptc(ch, "[%5d]   у %s в %s (%s)\r\n", 
                    room->vnum,
                    wch->getNameP('2').c_str( ), 
                    room->getName(), 
                    room->areaName().c_str());
        else if (o->in_room)
            ptc(ch, "[%5d]   на полу в %s (%s)\r\n", 
                    room->vnum,
                    room->getName(), 
                    room->areaName().c_str());
        else if (o->in_obj)
            ptc(ch, "[%5d]   внутри %s в %s (%s)\r\n", 
                    room->vnum,
                    o->in_obj->getShortDescr('2').c_str( ),
                    room->getName(),
                    room->areaName().c_str());
    }

    return true;
}
// Need to issue warning if flag isn't valid. -- does so now -- Hugin.
OEDIT(addaffect)
{
    OBJ_INDEX_DATA *pObj;
    Affect *pAf;
    int mod = 0, loc;
    const FlagTable *table = 0;
    GlobalRegistryBase *registry = 0;
    bitstring_t bit = 0;
    list<GlobalRegistryElement *> elements;
    DLString args = argument;
    DLString buf = args.getOneArgument();

    EDIT_OBJ(ch, pObj);

    if (buf.empty()) {
        stc("Syntax:  addaffect [location] [#mod] [table] [bit]\n\r", ch);
        return false;
    }

    if ((loc = apply_flags.value( buf )) == NO_FLAG) {        /* Hugin */
        stc("Valid locations are:\n\r", ch);
        show_help(ch, "apply");
        return false;
    }
    
    buf = args.getOneArgument();
    
    if(buf.empty() || !buf.isNumber()) {
        stc("Number expected after location identifier\n\r", ch);
        return false;
    }
    mod = atoi(buf.c_str());

    buf = args.getOneArgument();

    if(!buf.empty()) {
        table = FlagTableRegistry::getTable(buf);
        if (!table) {
            auto r = registryMap.find(buf);
            registry = r == registryMap.end() ? 0 : const_cast<GlobalRegistryBase *>(r->second);
        }

        if (!table && !registry) {
            stc("Valid table names are:\r\n", ch);
            stc("    detect_flags, affect_flags, vuln_flags, res_flags, imm_flags, weapon_type2, extra_flags\r\n", ch);
            stc("    skill, skillGroup, liquid, wearlocation\r\n", ch);
            return false;
        }

        if (table) {
            bit = table->bitstring(args);
            if (bit == NO_FLAG) {
                ptc(ch, "Invalid flag, see 'olchelp %s' for the list of values.\r\n", buf.c_str());
                return false;
            }
        } else {
            elements = registry->findAll(args);
            if (elements.empty()) {
                ptc(ch, "Elements with names \"%s\" not found in %s.\r\n", args.c_str(), buf.c_str());
                return false;
            }
        }
    }

    pAf = AffectManager::getThis()->getAffect();
    pAf->location = loc;
    pAf->location.setTable(&apply_flags);
    pAf->modifier = mod;
    pAf->type = -1;
    pAf->duration = -1;
    pAf->bitvector.setTable(table);
    pAf->bitvector.setValue(bit);
    pAf->global.setRegistry(registry);
    for (auto &e: elements)
        pAf->global.set(e->getIndex());

    pObj->affected.push_front(pAf);

    stc("Affect added.\n\r", ch);
    return true;
}

// My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
// for really teaching me how to manipulate pointers.
OEDIT(delaffect)
{
    OBJ_INDEX_DATA *pObj;
    Affect *pAf;
    char affect[MAX_STRING_LENGTH];
    int value;

    EDIT_OBJ(ch, pObj);

    one_argument(argument, affect);

    if (!is_number(affect) || affect[0] == '\0') {
        stc("Syntax:  delaffect [#xaffect]\n\r", ch);
        return false;
    }

    value = atoi(affect);

    if (value < 0) {
        stc("Only non-negative affect-numbers allowed.\n\r", ch);
        return false;
    }

    if (pObj->affected.empty()) {
        stc("OEdit: No affects found.\n\r", ch);
        return false;
    }

    pAf = pObj->affected.get(value);
    if (!pAf) {
        stc("No such affect.\n\r", ch);
        return false;
    }

    pObj->affected.remove(pAf);
    AffectManager::getThis()->extract(pAf);
    stc("Affect removed.\n\r", ch);
    return true;
}

OEDIT(random)
{
    OBJ_INDEX_DATA *pObj;
    DLString arg = DLString(argument).getOneArgument();
    Integer bestTier;

    EDIT_OBJ(ch, pObj);

    if (pObj->item_type != ITEM_WEAPON) {
        stc("Random stats can only be assigned to a weapon.\r\n", ch);
        return false;
    }

    if (arg.empty()) {
        stc("Usage:\r\nrandom <best tier> - mark this weapon as rand_stat\r\n", ch);
        stc("random clear - make this weapon non-random\r\n", ch);
        return false;
    }

    if (arg_is_clear(arg)) {
        Properties::iterator p = pObj->properties.find("random");
        if (p == pObj->properties.end()) {
            stc("This weapon is not random.\r\n", ch);
            return false;
        }

        pObj->properties.erase(p);
        stc("This weapon is no longer random.\r\n", ch);

        p = pObj->properties.find("bestTier");
        if (p != pObj->properties.end()) {
            pObj->properties.erase(p);
            stc("bestTier property is removed.\r\n", ch);
        }

        return true;
    }

    if (!Integer::tryParse(bestTier, arg)) {
        stc("Usage: random <best tier>r\n", ch);
        return false;
    }

    pObj->affected.deallocate();
    pObj->properties["random"] = "true";
    pObj->properties["bestTier"] = bestTier.toString();
    ptc(ch, "This weapon is now random, bestTier set to %d, affects removed.\r\n", bestTier.getValue());
    return true;
}

OEDIT(name)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return editor(argument, pObj->name, ED_NO_NEWLINE);
}

OEDIT(short)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return editor(argument, pObj->short_descr, ED_NO_NEWLINE);
}

OEDIT(long)
{
    return editor(argument, obj.description, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(sound)
{
    if (!*argument) {
        if(sedit(obj.sound)) {
            stc("Sound set\n\r", ch);
            return true;
        } else
            return false;
    }
    
    obj.sound = argument;
    stc("Sound set\n\r", ch);
    return false;
}

OEDIT(smell)
{
    if (!*argument) {
        if(sedit(obj.smell)) {
            stc("Smell set\n\r", ch);
            return true;
        } else
            return false;
    }
    
    obj.smell = argument;
    stc("Smell set\n\r", ch);
    return false;
}

OEDIT(property)
{
    DLString args = DLString( argument );
    return mapEdit( obj.properties, args );
}

bool set_value(Character * ch, OBJ_INDEX_DATA * pObj, char *argument, int value)
{
    if (argument[0] == '\0') {
        set_obj_values(ch, pObj, -1, "");        /* '\0' changed to "" -- Hugin */
        return false;
    }

    if (set_obj_values(ch, pObj, value, argument))
        return true;
    return false;
}

/*****************************************************************************
 Name:          oedit_values
 Purpose:       Finds the object and sets its value.
 Called by:     The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool OLCStateObject::oedit_values(Character * ch, char *argument, int value)
{
    if (set_value(ch, &obj, argument, value))
        return true;

    return false;
}


OEDIT(value0)
{
    return oedit_values(ch, argument, 0);
}

OEDIT(v0)
{
    return oedit_values(ch, argument, 0);
}

OEDIT(value1)
{
    return oedit_values(ch, argument, 1);
}

OEDIT(v1)
{
    return oedit_values(ch, argument, 1);
}

OEDIT(value2)
{
    return oedit_values(ch, argument, 2);
}

OEDIT(v2)
{
    return oedit_values(ch, argument, 2);
}

OEDIT(value3)
{
    return oedit_values(ch, argument, 3);
}

OEDIT(v3)
{
    return oedit_values(ch, argument, 3);
}

OEDIT(value4)
{
    return oedit_values(ch, argument, 4);
}

OEDIT(v4)
{
    return oedit_values(ch, argument, 4);
}

OEDIT(weight)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return numberEdit(0, 10000, pObj->weight);
}

OEDIT(cost)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return numberEdit(0, 1000000, pObj->cost);
}

OEDIT(create)
{
    AreaIndexData *pArea;
    int value;

    value = atoi(argument);
    if (argument[0] == '\0' || value == 0) {
        stc("Syntax:  oedit create [vnum]\n\r", ch);
        return false;
    }

    pArea = get_vnum_area(value);
    if (!pArea) {
        stc("OEdit:  That vnum is not assigned an area.\n\r", ch);
        return false;
    }

    if (!can_edit(ch, value)) {
        stc("OEdit:  Vnum in an area you cannot build in.\n\r", ch);
        return false;
    }

    if (get_obj_index(value)) {
        stc("OEdit:  Object vnum already exists.\n\r", ch);
        return false;
    }


    Pointer oe(NEW, value);
    oe->attach( ch );
    
    stc("Object Created.\n\r", ch);
    return true;
}

OEDIT(ed)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return extraDescrEdit(pObj->extra_descr);
}

OEDIT(extra)
{
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);
    return flagBitsEdit(extra_flags, pObj->extra_flags);
}


OEDIT(wear)
{
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);
    return flagBitsEdit(wear_flags, pObj->wear_flags);
}

OEDIT(type)
{
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);

    if (flagValueEdit(item_table, pObj->item_type)) {
        /*
         * Clear the values.
         */
        pObj->value[0] = 0;
        pObj->value[1] = 0;
        pObj->value[2] = 0;
        pObj->value[3] = 0;
        pObj->value[4] = 0;

        return true;
    }
    
    return false;
}

// TODO: check against hard-coded list of materials.
OEDIT(material)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0') {
        stc("Syntax:  material [string]\n\r", ch);
        return false;
    }

    free_string(pObj->material);
    pObj->material = str_dup(argument);

    stc("Material set.\n\r", ch);
    return true;
}

OEDIT(level)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return numberEdit(0, 120, pObj->level);
}

OEDIT(limit)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    return numberEdit(-1, 100, pObj->limit);
}

OEDIT(gender)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0') {
        stc("Syntax:  gender m|f|n|p\n\r", ch);
        return false;
    }

    pObj->gram_gender = Grammar::MultiGender(argument);

    ptc(ch, "Grammatical gender set to '%s'.\n\r", pObj->gram_gender.toString());
    return true;
}

OEDIT(condition)
{
    OBJ_INDEX_DATA *pObj;
    EDIT_OBJ(ch, pObj);
    return numberEdit(0, 100, pObj->condition); 
}

/*
 * XXX: theoretically, can be used to peek stats of items 
 * that you have no right to edit
 */
OEDIT(copy)
{
    OBJ_INDEX_DATA *original;
    DLString report;
    DLString args = DLString(argument);
    DLString arg1 = args.getOneArgument();
    DLString arg2 = args.getOneArgument();
    enum {
        COPY_DESC,
        COPY_PARAM,
        COPY_ERROR
    } mode;
    
    if (arg1.strPrefix("desc"))
        mode = COPY_DESC;
    else if (arg1.strPrefix("param"))
        mode = COPY_PARAM;
    else 
        mode = COPY_ERROR;
            
    if (mode == COPY_ERROR || !arg2.isNumber()) {
        ch->pecho("Syntax: \r\n"
                    "  copy param <vnum> -- copy affects, level, flags and other parameters from <vnum> obj index.\r\n"
                    "  copy desc <vnum>  -- copy name, short, long and extra descriptions from <vnum> obj index.\r\n" );
        return false;
    }
    
    original = get_obj_index( arg2.toInt() );

    if (original == NULL) {
        ch->pecho("Object not found.\r\n");
        return false;
    }
    
    switch (mode) {
    case COPY_PARAM:
        copyParameters( original );
        report = "parameters";
        break;
    case COPY_DESC:
        copyDescriptions( original );
        report = "descriptions";
        break;
    default:
        return false;
    }
    
    ch->printf("All %s copied from vnum %d (%s).\r\n",
                report.c_str( ),
                original->vnum, 
                russian_case( original->short_descr, '1' ).c_str( ) );
    return true;
}

OEDIT(list)
{
    int cnt;
    RoomIndexData *pRoom;
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    ostringstream buffer;
    
    EDIT_OBJ(ch, pObj);
    
    snprintf(buf, sizeof(buf), "Resets for object [{W%d{x] ({g%s{x):\n\r",
            pObj->vnum, 
            russian_case(pObj->short_descr, '1').c_str( ));
    buffer << buf;
    
    cnt = 0;
    for (auto &r: roomIndexMap) {
        pRoom = r.second;
        for(auto &pReset: pRoom->resets)
            switch(pReset->command) {
                case 'G':
                case 'E':
                case 'O':
                case 'P':
                    if(pReset->arg1 == pObj->vnum) {
                        snprintf(buf, sizeof(buf), "{G%c{x in room [{W%d{x] ({g%s{x)\n\r",
                                pReset->command, pRoom->vnum, pRoom->name);
                        buffer << buf;
                        cnt++;
                    }
            }
    }

    snprintf(buf, sizeof(buf), "Total {W%d{x resets found.\n\r", cnt);
    buffer << buf;
    
    page_to_char( buffer.str( ).c_str( ), ch );
    return false;
}

OEDIT(behavior)
{
    if (argument[0] == '\0') {
        if(!xmledit(obj.behavior))
            return false;

        stc("Behavior set.\r\n", ch);
        return true;
    }

    if (!str_cmp( argument, "clear" )) {        
        obj.behavior.clear( );
        stc("Поведение очищено.\r\n", ch);
        return true;
    }

    stc("Syntax:  behavior       - line edit\n\r", ch);
    stc("Syntax:  behavior clear\n\r", ch);
    return false;
}

OEDIT(commands)
{
    do_commands(ch);
    return false;
}

OEDIT(done) 
{
    commit();
    detach(ch);
    return true;
}

OEDIT(cancel)
{
    detach(ch);
    return false;
}

OEDIT(dump)
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}


CMD(oedit, 50, "", POS_DEAD, 103, LOG_ALWAYS,
        "Online object editor.")
{
    Object *obj;
    OBJ_INDEX_DATA *pObj;
    AreaIndexData *pArea;
    char arg1[MAX_STRING_LENGTH];
    int value;

    argument = one_argument(argument, arg1);

    if (is_number(arg1)) {
        value = atoi(arg1);
        if (!(pObj = get_obj_index(value))) {
            stc("OEdit:  That vnum does not exist.\n\r", ch);
            return;
        }

        if(!pObj->area) {
            stc("this object has no area!!!!!\n\r", ch);
            return;
        }

        if (!OLCState::can_edit(ch, pObj->vnum)) {
            stc("У тебя недостаточно прав для редактирования предметов.\n\r", ch);
            return;
        }

        OLCStateObject::Pointer oe(NEW, pObj);
        oe->attach(ch);
        oe->findCommand(ch, "show")->run(ch, "");
        return;
    } else if (!str_cmp(arg1, "create")) {
        if (!str_cmp(argument, "next")) {
            value = next_obj_index(ch, ch->in_room->pIndexData);
            if (value < 0) {
                ch->pecho("Все внумы в этой зоне уже заняты!");
                return;
            }
        }
        else
            value = atoi(argument);

        if (argument[0] == '\0' || value <= 0) {
            stc("Syntax:  edit object create <vnum>|next\n\r", ch);
            return;
        }

        pArea = OLCState::get_vnum_area(value);

        if (!pArea) {
            stc("OEdit:  That vnum is not assigned an area.\n\r", ch);
            return;
        }

        if (!OLCState::can_edit(ch, value)) {
            stc("У тебя недостаточно прав для редактирования предметов.\n\r", ch);
            return;
        }

        if (get_obj_index(value)) {
            stc("OEdit:  Object vnum already exists.\n\r", ch);
            return;
        }

        OLCStateObject::Pointer oe(NEW, value);
        oe->attach(ch);
        return;
    } else if (!str_cmp(arg1, "delete")) {
        OBJ_INDEX_DATA *obji = NULL;

        value = atoi(argument);
        if (argument[0] == '\0' || value == 0) {
            stc("Syntax:  edit object delete [vnum]\n\r", ch);
            return;
        }

        pArea = OLCState::get_vnum_area(value);

        if (!pArea) {
            stc("OEdit:  That vnum is not assigned an area.\n\r", ch);
            return;
        }

        if (!OLCState::can_edit(ch, value)) {
            stc("У тебя недостаточно прав для редактирования предметов.\n\r", ch);
            return;
        }

        obji = get_obj_index(value);

        if (obji) {
            pArea->changed = true;
            SET_BIT(obji->extra_flags, ITEM_DELETED);
            ptc(ch, "[%u] (%s) помечен к удалению.\n\r", obji->vnum, russian_case(obji->short_descr, '1').c_str());
        }
        else
            ptc(ch, "Предмет %d не найден.\n\r", value);
        return;

    } else if(!str_cmp(arg1, "show")) {
        if(!*argument || !is_number(argument)) {
            stc("Syntax: oedit show <vnum>\n\r", ch);
            return;
        }
        pObj = get_obj_index(atoi(argument));
        if(!pObj) {
            stc("Нет такого объекта.\n\r", ch);
            return;
        }
        
        if (!OLCState::can_edit(ch, pObj->vnum)) {
            stc("У тебя недостаточно прав для редактирования объектов.\n\r", ch);
            return;
        }

        OLCStateObject::Pointer(NEW, pObj)->findCommand(ch, "show")->run(ch, "noweb");
        return;
    } else if (!str_cmp(arg1, "load")) {
        if(!*argument || !is_number(argument)) {
            stc("Syntax: oedit load <vnum>\n\r", ch);
            return;
        }
        pObj = get_obj_index(atoi(argument));
        if(!pObj) {
            stc("Нет такого объекта.\n\r", ch);
            return;
        }
        
        if (!OLCState::can_edit(ch, pObj->vnum)) {
            stc("У тебя недостаточно прав для создания этого предмета.\n\r", ch);
            return;
        }
        
        obj = create_object( pObj, ch->get_trust( ) );
        
        if ( obj->can_wear( ITEM_TAKE) )
            obj_to_char( obj, ch );
        else
            obj_to_room( obj, ch->in_room );

        oldact("$c1 создает $o4!", ch, obj, 0, TO_ROOM);
        oldact("Ты создаешь $o4!", ch, obj, 0, TO_CHAR);
        return;
    }
    stc("OEdit:  There is no default object to edit.\n\r", ch);
}

void OLCStateObject::changed( PCharacter *ch )
{
    if(obj.area)
        obj.area->changed = true;
}


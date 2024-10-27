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
#include "string_utils.h"
#include "json_utils_ext.h"
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
#include "behavior.h"

#include "loadsave.h"

#include "olc.h"
#include "security.h"

#include "def.h"

#define EDIT_OBJ(x, y) y = &obj

OLC_STATE(OLCStateObject);

BHV(random_weapon);

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
    copyBehaviors(original);
}

OLCStateObject::OLCStateObject( int vnum )
{
    obj.vnum = vnum;
    obj.area = get_vnum_area(vnum);
}

OLCStateObject::~OLCStateObject( )
{
}

void OLCStateObject::copyBehaviors(OBJ_INDEX_DATA *original)
{
    obj.behaviors.clear();
    obj.behaviors.set(original->behaviors);
    obj.props.clear();
    JsonUtils::copy(obj.props, original->props);
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
    for (auto &ed: original->extraDescriptions) {
        ExtraDescription *myed = new ExtraDescription();
        myed->keyword = ed->keyword;
        myed->description = ed->description;
        obj.extraDescriptions.push_back(myed);
    }

    obj.keyword      = original->keyword;
    obj.short_descr  = original->short_descr;
    obj.description  = original->description;
    obj.smell        = original->smell;
    obj.sound        = original->sound;
    obj.material     = str_dup(original->material);
    obj.gram_gender  = original->gram_gender;
}

void OLCStateObject::commit()
{
    Object *o;
    OBJ_INDEX_DATA *original;

    original = get_obj_index(obj.vnum);

    if(!original) {
        int iHash;
        
        original = new OBJ_INDEX_DATA;

        original->vnum = obj.vnum;
        original->area = obj.area;

        iHash = (int) obj.vnum % MAX_KEY_HASH;
        original->next = obj_index_hash[iHash];
        obj_index_hash[iHash] = original;
    }
    
    original->extraDescriptions.deallocate();
    for (auto &ed: obj.extraDescriptions) {
        ExtraDescription *myed = new ExtraDescription();
        myed->keyword = ed->keyword;
        myed->description = ed->description;
        original->extraDescriptions.push_back(myed);        
    }
    obj.extraDescriptions.clear();
    
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
    
    original->keyword = obj.keyword;
    original->short_descr = obj.short_descr;
    original->description = obj.description;
    original->sound        = obj.sound;
    original->smell        = obj.smell;
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

    original->behaviors.clear();
    original->behaviors.set(obj.behaviors);
    original->props.clear();
    JsonUtils::copy(original->props, obj.props);

    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original) {
            o->updateCachedNouns( );
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
    int cnt = 0;
    bool showWeb = !arg_is_strict(argument, "noweb");

    EDIT_OBJ(ch, pObj);

    ptc(ch, "Vnum:     [{W%d{x] Area: [{W%d{x] {W%s{x\n\r", pObj->vnum, pObj->area->vnum, pObj->area->getName().c_str());
    ptc(ch, "Name:     [{W%s{x] %s [{W%s{x] %s [{W%s{x] %s\n\r", 
            pObj->keyword[EN].c_str(), web_edit_button(showWeb, ch, "name", "web").c_str(),
            pObj->keyword[UA].c_str(), web_edit_button(showWeb, ch, "uaname", "web").c_str(),
            pObj->keyword[RU].c_str(), web_edit_button(showWeb, ch, "runame", "web").c_str());

    ptc(ch, "Short EN: [%s] %s\n\r", pObj->short_descr[EN].c_str(), web_edit_button(showWeb, ch, "short", "web").c_str());
    ptc(ch, "Short UA: [%s] %s\n\r", pObj->short_descr[UA].c_str(), web_edit_button(showWeb, ch, "uashort", "web").c_str());
    ptc(ch, "Short RU: [%s] %s\n\r", pObj->short_descr[RU].c_str(), web_edit_button(showWeb, ch, "rushort", "web").c_str());
    ptc(ch, "Long EN:  [%s] %s\n\r", pObj->description[EN].c_str(), web_edit_button(showWeb, ch, "long", "web").c_str());
    ptc(ch, "Long UA:  [%s] %s\n\r", pObj->description[UA].c_str(), web_edit_button(showWeb, ch, "ualong", "web").c_str());
    ptc(ch, "Long RU:  [%s] %s\n\r", pObj->description[RU].c_str(), web_edit_button(showWeb, ch, "rulong", "web").c_str());
    ptc(ch, "Type:     [%s] {D(? item_table){x\n\r", item_table.name(pObj->item_type).c_str());
    ptc(ch, "Level:    [%5d]\n\r", pObj->level);    
    ptc(ch, "Limit:    [%5d]\n\r", pObj->limit);
    ptc(ch, "Wear:     [%s] {D(? wear_flags){x\n\r", wear_flags.names(pObj->wear_flags).c_str());
    ptc(ch, "Extra:    [%s] {D(? extra_flags){x\n\r", extra_flags.names(pObj->extra_flags).c_str());
    ptc(ch, "Material: [%s] {D(? material){x\n\r", pObj->material);
    ptc(ch, "Smell:    [{W%s{x] %s [{W%s{x] %s [{W%s{x] %s\n\r", 
          String::stripEOL(pObj->smell.get(EN)).c_str(), web_edit_button(showWeb, ch, "smell", "web").c_str(),   
          String::stripEOL(pObj->smell.get(UA)).c_str(), web_edit_button(showWeb, ch, "uasmell", "web").c_str(),   
          String::stripEOL(pObj->smell.get(RU)).c_str(), web_edit_button(showWeb, ch, "rusmell", "web").c_str());   
    ptc(ch, "Sound:    [{W%s{x] %s [{W%s{x] %s [{W%s{x] %s\n\r", 
          String::stripEOL(pObj->sound.get(EN)).c_str(), web_edit_button(showWeb, ch, "sound", "web").c_str(),   
          String::stripEOL(pObj->sound.get(UA)).c_str(), web_edit_button(showWeb, ch, "uasound", "web").c_str(),   
          String::stripEOL(pObj->sound.get(RU)).c_str(), web_edit_button(showWeb, ch, "rusound", "web").c_str());   

    ptc(ch, "Condition:[%5d]\n\r", pObj->condition);
    ptc(ch, "Gender:   [%1s]\n\r", pObj->gram_gender.toString());
    ptc(ch, "Weight:   [%5d]\n\r", pObj->weight);
    
    if (!pObj->behaviors.isSet(bhv_random_weapon))
        ptc(ch, "Cost:     [%5d]\n\r", pObj->cost);

    if (!pObj->extraDescriptions.empty()) {

        ptc(ch, "Extra desc: {D(ed help){x\n\r");
        for (auto &ed: pObj->extraDescriptions) {
            ptc(ch, "    EN [%s] %s \r\n", ed->keyword.c_str(), web_edit_button(showWeb, ch, "ed web", ed->keyword).c_str());
            ptc(ch, "    UA [%s] %s \r\n", ed->keyword.c_str(), web_edit_button(showWeb, ch, "uaed web", ed->keyword).c_str());
            ptc(ch, "    RU [%s] %s \r\n", ed->keyword.c_str(), web_edit_button(showWeb, ch, "rued web", ed->keyword).c_str());
        }

    } else {
        ptc(ch, "Extra desc: (none) {D(ed help){x\r\n");
    }

    for (auto &paf: pObj->affected) {
        if (cnt == 0) {
            stc("Number  Location  Modifier  Table/Registry  Bits\n\r", ch);
            stc("------  --------  --------  --------------  ---------------------------\n\r", ch);
        }

        ptc(ch, "[%4d]  %-8.8s  %8d", cnt,
                  paf->location.name().c_str(),
                  paf->modifier.getValue());
       
        const FlagTable *table = paf->bitvector.getTable();
        const GlobalRegistryBase *registry = paf->global.getRegistry();

        if ((table && paf->bitvector.getValue()) || registry) { 
            ptc(ch, "  %-14.14s  ",
                      registry ? registry->getRegistryName().c_str() : paf->bitvector.getTableName().c_str());

            if (registry) {
                ptc(ch, "%s", paf->global.toString().c_str());

                if (registry == liquidManager) {
                    ptc(ch, " {D(? liquid){x");
                } else if (registry == skillManager) {
                                        
                } else if (registry == skillGroupManager) {
                    ptc(ch, " {D(? practicer){x");
                } else if (registry == wearlocationManager) {
                    ptc(ch, " {D(? wearloc){x");
                } else {
                    ptc(ch, "<unknown registry>");
                }

            } else {
                ptc(ch, paf->bitvector.names().c_str());
                ptc(ch, " {D(? %s){x", paf->bitvector.getTableName().c_str());
            }
        }

        ptc(ch, "\n\r");
        cnt++;
    }
    if (!pObj->affected.empty())
        stc("{D        ? apply_flags{x\r\n", ch);

    show_obj_values(ch, pObj);

    if (pObj->behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            pObj->behavior->save( ostr );
            ptc(ch, "Legacy behavior: {D(oldbehavior{x)\r\n%s\r\n", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Legacy behavior is BUGGY.\r\n");
        }
    }

    show_behaviors(ch, pObj->behaviors, pObj->props);

    // Display Fenia triggers and methods on this object.
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
    ch->pecho("%N1 находится:", obj.getShortDescr(LANG_DEFAULT));
    
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
            ptc(ch, "[%5d]   внутри %N2 в %s (%s)\r\n", 
                    room->vnum,
                    o->in_obj->getShortDescr(LANG_DEFAULT).c_str( ),
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

        if (!pObj->behaviors.isSet(bhv_random_weapon)) {
            stc("This weapon is not random.\r\n", ch);
            return false;
        }

        pObj->behaviors.remove(bhv_random_weapon);
        pObj->props.removeMember(bhv_random_weapon->getName());

        stc("This weapon is no longer random.\r\n", ch);
        return true;
    }

    if (!Integer::tryParse(bestTier, arg)) {
        stc("Usage: random <best tier>r\n", ch);
        return false;
    }

    pObj->affected.deallocate();
    pObj->behaviors.set(bhv_random_weapon);
    pObj->props[bhv_random_weapon->getName()]["random"] = "true";
    pObj->props[bhv_random_weapon->getName()]["bestTier"] = bestTier.toString();
    
    ptc(ch, "This weapon is now random, bestTier set to %d, affects removed.\r\n", bestTier.getValue());
    return true;
}

OEDIT(name)
{
    return editor(argument, obj.keyword[EN], ED_NO_NEWLINE);
}

OEDIT(uaname)
{
    return editor(argument, obj.keyword[UA], ED_NO_NEWLINE);
}

OEDIT(runame)
{
    return editor(argument, obj.keyword[RU], ED_NO_NEWLINE);
}

OEDIT(short)
{
    return editor(argument, obj.short_descr[EN], ED_NO_NEWLINE);
}

OEDIT(uashort)
{
    return editor(argument, obj.short_descr[UA], ED_NO_NEWLINE);
}

OEDIT(rushort)
{
    return editor(argument, obj.short_descr[RU], ED_NO_NEWLINE);
}

OEDIT(long)
{
    return editor(argument, obj.description[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(ualong)
{
    return editor(argument, obj.description[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(rulong)
{
    return editor(argument, obj.description[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(sound)
{
    return editor(argument, obj.sound[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(uasound)
{
    return editor(argument, obj.sound[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(rusound)
{
    return editor(argument, obj.sound[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(smell)
{
    return editor(argument, obj.smell[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(uasmell)
{
    return editor(argument, obj.smell[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(rusmell)
{
    return editor(argument, obj.smell[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_NO_NEWLINE));
}

OEDIT(props)
{
    return editProps(obj.behaviors, obj.props, argument);
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
    return numberEdit(0, 10000, obj.weight);
}

OEDIT(cost)
{
    return numberEdit(0, 1000000, obj.cost);
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
    return extraDescrEdit(obj.extraDescriptions);
}

OEDIT(uaed)
{
    return extraDescrEdit(obj.extraDescriptions);
}

OEDIT(rued)
{
    return extraDescrEdit(obj.extraDescriptions);
}

OEDIT(extra)
{
    return flagBitsEdit(extra_flags, obj.extra_flags);
}


OEDIT(wear)
{
    return flagBitsEdit(wear_flags, obj.wear_flags);
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
    return numberEdit(0, 120, obj.level);
}

OEDIT(limit)
{
    return numberEdit(-1, 100, obj.limit);
}

OEDIT(gender)
{
    return genderEdit(obj.gram_gender);
}

OEDIT(condition)
{
    return numberEdit(0, 100, obj.condition); 
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
        COPY_BHV,
        COPY_ERROR
    } mode;
    
    if (arg_is(arg1, "desc"))
        mode = COPY_DESC;
    else if (arg_is(arg1, "param"))
        mode = COPY_PARAM;
    else if (arg_is(arg1, "behavior"))
        mode = COPY_BHV;
    else 
        mode = COPY_ERROR;
            
    if (mode == COPY_ERROR || !arg2.isNumber()) {
        ch->pecho("Syntax: \r\n"
                    "  copy param <vnum> -- copy affects, level, flags and other parameters from <vnum> obj index.\r\n"
                    "  copy behav <vnum> -- copy behaviors and props.\r\n"
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
    case COPY_BHV:
        copyBehaviors(original);
        report = "behaviors";
        break;
    default:
        return false;
    }
    
    ch->pecho("All %s copied from vnum %d (%N1).",
                report.c_str( ),
                original->vnum, 
                original->getShortDescr(LANG_DEFAULT));
    return true;
}

OEDIT(list)
{
    int cnt;
    RoomIndexData *pRoom;
    OBJ_INDEX_DATA *pObj;
    ostringstream buffer;
    
    EDIT_OBJ(ch, pObj);
    
    buffer << fmt(0, "Resets for object [{W%d{x] ({g%N1{x):\n\r",
            pObj->vnum, 
            pObj->getShortDescr(LANG_DEFAULT));
    
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
                        buffer << fmt(0, "{G%c{x in room [{W%d{x] ({g%s{x)\n\r",
                                pReset->command, pRoom->vnum, pRoom->name.get(LANG_DEFAULT).c_str());
                        cnt++;
                    }
            }
    }

    buffer << fmt(0, "Total {W%d{x resets found.\n\r", cnt);
    
    page_to_char( buffer.str( ).c_str( ), ch );
    return false;
}

OEDIT(behaviors)
{
    return editBehaviors(obj.behaviors, obj.props);
}

OEDIT(oldbehavior)
{
    if (argument[0] == '\0') {
        if(!xmledit(obj.behavior))
            return false;

        stc("Legacy behavior set.\r\n", ch);
        return true;
    }

    if (!str_cmp( argument, "clear" )) {        
        obj.behavior.clear( );
        stc("Старое поведение очищено.\r\n", ch);
        return true;
    }

    stc("Syntax:  oldbehavior       - line edit\n\r", ch);
    stc("Syntax:  oldbehavior clear\n\r", ch);
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
        oe->findCommand(ch, "show")->entryPoint(ch, "");
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
            ptc(ch, "[%u] (%N1) помечен к удалению.\n\r", obji->vnum, obji->getShortDescr(LANG_DEFAULT));
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

        OLCStateObject::Pointer(NEW, pObj)->findCommand(ch, "show")->entryPoint(ch, "noweb");
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


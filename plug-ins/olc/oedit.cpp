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
#include "room.h"

#include "oedit.h"
#include "ovalues.h"
#include "comm.h"
#include "merc.h"
#include "interp.h"

#include "act.h"
#include "mercdb.h"
#include "handler.h"

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
    obj.new_format   = original->new_format;
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
    Affect *af, **my_af = &obj.affected;
    list<Affect *> afflist;
    for(af = original->affected; af; af = af->next)
        afflist.push_back(af);

    while(!afflist.empty()) {
        af = afflist.back();
        afflist.pop_back();
        
        *my_af = dallocate( Affect );
        (*my_af)->where    =af->where; 
        (*my_af)->type     =af->type; 
        (*my_af)->level    =af->level;
        (*my_af)->duration =af->duration;
        (*my_af)->location =af->location;
        (*my_af)->modifier =af->modifier;
        (*my_af)->bitvector=af->bitvector;
        my_af = &(*my_af)->next;
    }
    *my_af = 0;
    
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
    
    Affect *af, *af_next;
    for(af = original->affected; af; af = af_next) {
        af_next = af->next;
        ddeallocate( af );
    }
    original->affected = obj.affected;
    obj.affected = 0;
    
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
            
            if(!memcmp(o->value, original->value, sizeof(o->value)))
                memcpy(o->value, obj.value, sizeof(o->value));
        }
    
    original->new_format   = obj.new_format;
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

    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original) 
            o->updateCachedNoun( );
    
    original->properties.clear( );
    for (Properties::const_iterator p = obj.properties.begin( ); p != obj.properties.end( ); p++)
        original->properties.insert( *p );
    obj.properties.clear( );
}

void OLCStateObject::statePrompt(Descriptor *d)
{
    d->send( "Editing obj> " );
}

OEDIT(show)
{
    OBJ_INDEX_DATA *pObj;
    char buf[MAX_STRING_LENGTH];
    Affect *paf;
    int cnt;

    EDIT_OBJ(ch, pObj);

    sprintf(buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
              pObj->name,
              !pObj->area ? -1 : pObj->area->vnum,
              !pObj->area ? "No Area" : pObj->area->name);
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
            ptc(ch, "%20s: %s\n\r", p->first.c_str( ), p->second.c_str( ));
    }

    ptc(ch, "Condition:   [%5d]\n\r", pObj->condition);

    ptc(ch, "Gender:      [%1s]\n\r", pObj->gram_gender.toString());

    sprintf(buf, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
              pObj->weight, pObj->cost);
    stc(buf, ch);

    if (pObj->extra_descr) {
        EXTRA_DESCR_DATA *ed;

        stc("Ex desc kwd: ", ch);

        for (ed = pObj->extra_descr; ed; ed = ed->next) {
            stc("[", ch);
            stc(ed->keyword, ch);
            stc("]", ch);
        }

        stc("\n\r", ch);
    }

    sprintf(buf, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r",
              pObj->short_descr, pObj->description);
    stc(buf, ch);

    for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
        if (cnt == 0) {
            stc("Number Modifier Location Where      What\n\r", ch);
            stc("------ -------- -------- ---------- ---------------------------\n\r", ch);
        }
        sprintf(buf, "[%4d] %-8d %-8.8s", cnt,
                  paf->modifier,
                  apply_flags.name(paf->location).c_str());
       
        if (paf->bitvector) { 
            sprintf(buf+strlen(buf), " %-10.10s ",
                      affwhere_flags.name(paf->where).c_str());

            switch(paf->where) {
                case TO_DETECTS:
                    strcat(buf, detect_flags.names(paf->bitvector).c_str());
                    strcat(buf, " {D(? detect_flags){x");
                    break;
                case TO_AFFECTS:
                    strcat(buf, affect_flags.names(paf->bitvector).c_str());
                    strcat(buf, " {D(? affect_flags){x");
                    break;
                case TO_IMMUNE:
                    strcat(buf, imm_flags.names(paf->bitvector).c_str());
                    strcat(buf, " {D(? imm_flags){x");
                    break;
                case TO_RESIST:
                    strcat(buf, res_flags.names(paf->bitvector).c_str());
                    strcat(buf, " {D(? res_flags){x");
                    break;
                case TO_VULN:
                    strcat(buf, vuln_flags.names(paf->bitvector).c_str());
                    strcat(buf, " {D(? vuln_flags){x");
                    break;
                default:
                    sprintf(buf + strlen(buf), "<%08x>", paf->bitvector);
                    break;
            }
        }
        strcat(buf, "\n\r");
        stc(buf, ch);
        cnt++;
    }
    if (pObj->affected)
        stc("{D          ? apply_flags  ? affwhere_flags{x\r\n", ch);

    show_obj_values(ch, pObj);

    if (pObj->behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            pObj->behavior->save( ostr );
            ptc(ch, "Behavior:\r\n%s\r\n", ostr.str( ).c_str( ));
            
        } catch (ExceptionXMLError e) {
            ptc(ch, "Behavior is BUGGY.\r\n");
        }
    }
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
                    room->name, 
                    room->area->name);
        else if (o->in_room)
            ptc(ch, "[%5d]   на полу в %s (%s)\r\n", 
                    room->vnum,
                    room->name, 
                    room->area->name);
        else if (o->in_obj)
            ptc(ch, "[%5d]   внутри %s в %s (%s)\r\n", 
                    room->vnum,
                    o->in_obj->getShortDescr('2').c_str( ),
                    room->name,
                    room->area->name);
    }

    return true;
}
// Need to issue warning if flag isn't valid. -- does so now -- Hugin.
OEDIT(addaffect)
{
    OBJ_INDEX_DATA *pObj;
    Affect *pAf;
    char buf[MAX_STRING_LENGTH];
    int where = 0, mod = 0, loc;
    bitstring_t bit = 0;

    EDIT_OBJ(ch, pObj);

    argument = one_argument(argument, buf);

    if (!*buf) {
        stc("Syntax:  addaffect [location] [#mod] [where] [bit]\n\r", ch);
        return false;
    }

    if ((loc = apply_flags.value( buf )) == NO_FLAG) {        /* Hugin */
        stc("Valid locations are:\n\r", ch);
        show_help(ch, "apply");
        return false;
    }
    
    argument = one_argument(argument, buf);
    if(!*buf || !is_number(buf)) {
        stc("Number expected after location identifier\n\r", ch);
        return false;
    }
    mod = atoi(buf);

    argument = one_argument(argument, buf);
    if(*buf) {
        if ((where = affwhere_flags.value( buf )) == NO_FLAG) {
            stc("Valid where fields are:\n\r", ch);
            show_help(ch, "affwhere");
            return false;
        }
        switch(where) {
            case TO_DETECTS:
                bit = detect_flags.bitstring( argument );
                if(bit == NO_FLAG) {
                    stc("Valid detects are:\n\r", ch);
                    show_help(ch, "detect" );
                    return false;
                }
                break;
            case TO_AFFECTS:
                bit = affect_flags.bitstring( argument );
                if(bit == NO_FLAG) {
                    stc("Valid affects are:\n\r", ch);
                    show_help(ch, "affect" );
                    return false;
                }
                break;
            case TO_IMMUNE:
                bit = imm_flags.bitstring( argument );
                if(bit == NO_FLAG) {
                    stc("Valid immun bits are:\n\r", ch);
                    show_help(ch, "imm" );
                    return false;
                }
                break;
            case TO_RESIST:
                bit = res_flags.bitstring( argument );
                if(bit == NO_FLAG) {
                    stc("Valid resist bits are:\n\r", ch);
                    show_help(ch, "res" );
                    return false;
                }
                break;
            case TO_VULN:
                bit = vuln_flags.bitstring( argument );
                if(bit == NO_FLAG) {
                    stc("Valid vulnerable bits are:\n\r", ch);
                    show_help(ch, "vuln" );
                    return false;
                }
                break;
            default:
                stc("This affect location is  not supported now.\n\r", ch);
                return false;
                break;
        }
    }

    pAf = new_affect();
    pAf->location = loc;
    pAf->modifier = mod;
    pAf->type = -1;
    pAf->duration = -1;
    pAf->where = where;
    pAf->bitvector = bit;
    pAf->next = pObj->affected;
    pObj->affected = pAf;

    stc("Affect added.\n\r", ch);
    return true;
}

// My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
// for really teaching me how to manipulate pointers.
OEDIT(delaffect)
{
    OBJ_INDEX_DATA *pObj;
    Affect *pAf;
    Affect *pAf_next;
    char affect[MAX_STRING_LENGTH];
    int value;
    int cnt = 0;

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

    if (!(pAf = pObj->affected)) {
        stc("OEdit:  Non-existant affect.\n\r", ch);
        return false;
    }

    if (value == 0) {                /* First case: Remove first affect */
        pAf = pObj->affected;
        pObj->affected = pAf->next;
        free_affect(pAf);
    }
    else {                        /* Affect to remove is not the first */
        while ((pAf_next = pAf->next) && (++cnt < value))
            pAf = pAf_next;

        if (pAf_next) {                /* See if it's the next affect */
            pAf->next = pAf_next->next;
            free_affect(pAf_next);
        }
        else {                        /* Doesn't exist */
            stc("No such affect.\n\r", ch);
            return false;
        }
    }

    stc("Affect removed.\n\r", ch);
    return true;
}

OEDIT(name)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0') {
        stc("Syntax:  name [string]\n\r", ch);
        return false;
    }

    free_string(pObj->name);
    pObj->name = str_dup(argument);

    stc("Name set.\n\r", ch);
    return true;
}

OEDIT(short)
{
    OBJ_INDEX_DATA *pObj;

    EDIT_OBJ(ch, pObj);

    if (argument[0] == '\0') {
        stc("Syntax:  short [string]\n\r", ch);
        return false;
    }

    free_string(pObj->short_descr);
    pObj->short_descr = str_dup(argument);

    stc("Short description set.\n\r", ch);
    return true;
}

OEDIT(long)
{
    if (!*argument) {
        if(sedit(obj.description)) {
            stc("Description set\n\r", ch);
            return true;
        } else
            return false;
    }

    free_string(obj.description );
    obj.description = str_dup(argument);
    *obj.description = Char::upper(*obj.description);

    stc("Description set\n\r", ch);
    return false;
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
    AREA_DATA *pArea;
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
#warning нет material_lookup

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
        ch->println("Syntax: \r\n"
                    "  copy param <vnum> -- copy affects, level, flags and other parameters from <vnum> obj index.\r\n"
                    "  copy desc <vnum>  -- copy name, short, long and extra descriptions from <vnum> obj index.\r\n" );
        return false;
    }
    
    original = get_obj_index( arg2.toInt() );

    if (original == NULL) {
        ch->println("Object not found.\r\n");
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
    int i, cnt;
    Room *pRoom;
    OBJ_INDEX_DATA *pObj;
    RESET_DATA *pReset;
    char buf[MAX_STRING_LENGTH];
    ostringstream buffer;
    
    EDIT_OBJ(ch, pObj);
    
    snprintf(buf, sizeof(buf), "Resets for object [{W%d{x] ({g%s{x):\n\r",
            pObj->vnum, 
            russian_case(pObj->short_descr, '1').c_str( ));
    buffer << buf;
    
    cnt = 0;
    for(i=0;i<MAX_KEY_HASH;i++)
        for(pRoom = room_index_hash[i];pRoom;pRoom = pRoom->next) 
            for(pReset = pRoom->reset_first;pReset;pReset = pReset->next)
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

    stc("Syntax:  behavior    - line edit\n\r", ch);
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
    AREA_DATA *pArea;
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
        return;
    } else if (!str_cmp(arg1, "create")) {
        if (!str_cmp(argument, "next")) 
            value = next_obj_index(ch, ch->in_room);
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
            SET_BIT(pArea->area_flag, AREA_CHANGED);
            SET_BIT(obji->extra_flags, ITEM_DELETED);
            ptc(ch, "[%u] (%s) marked as deleted.\n\r", obji->vnum, obji->name);
        }
        else
            stc("Item is not exist.\n\r", ch);
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

        OLCStateObject::Pointer(NEW, pObj)->findCommand(ch, "show")->run(ch, "");
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

        act_p( "$c1 создает $o4!", ch, obj, 0, TO_ROOM,POS_RESTING );
        act_p( "Ты создаешь $o4!", ch, obj, 0, TO_CHAR,POS_RESTING );
        return;
    }
    stc("OEdit:  There is no default object to edit.\n\r", ch);
}

void OLCStateObject::changed( PCharacter *ch )
{
    if(obj.area)
        SET_BIT(obj.area->area_flag, AREA_CHANGED);
}


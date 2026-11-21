/* $Id$
 *
 * ruffina, 2004
 */

#include <algorithm>
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
#include "wearlocation.h"
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
#include "occupations.h"
#include "itemeconomy.h"

#include "olc.h"
#include "olcvalidate.h"
#include "itemutils.h"
#include "keyhole.h"
#include "security.h"

#include "tohit.h"
#include "def.h"

#define EDIT_OBJ(x, y) y = &obj

OLC_STATE(OLCStateObject);

BHV(random_weapon);

// Furniture as bodyposition sees it: sit/rest/sleep on furniture and fountains.
static bool is_furniture(OBJ_INDEX_DATA *pObj)
{
    return pObj->item_type == ITEM_FURNITURE || pObj->item_type == ITEM_FOUNTAIN;
}

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
    obj.xml_weight   = original->xml_weight;
    obj.xml_cost     = original->xml_cost;
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
    obj.material     = original->material;
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
    
    // Worn instances carry the prototype's affects on their wearer: take the old
    // list off before it goes, or removing the item later subtracts the new one.
    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original && o->carried_by)
            o->wear_loc->affectsOnUnequip(o->carried_by, o);

    original->affected.deallocate();
    original->affected.assign(obj.affected.begin(), obj.affected.end());
    obj.affected.clear();
    
    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original && o->carried_by)
            o->wear_loc->affectsOnEquip(o->carried_by, o);

    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original) {
            /*object instance hase separate extra descr*/
            
            if(o->item_type == original->item_type)
                o->item_type = obj.item_type;
            
            if(o->extra_flags == original->extra_flags)
                o->extra_flags = obj.extra_flags;

            if(o->wear_flags == original->wear_flags)
                o->wear_flags   = obj.wear_flags;

            if(o->level == original->level)
                o->level = obj.level;
            
            if(o->condition == original->condition)
                obj_set_condition(o, obj.condition);
           
            for (int i = 0; i < 5; i++)
                if (o->getsValueFromProto(i) && o->valueByIndex(i) == original->value[i])
                    o->valueByIndex(i, obj.value[i]);
        }
    
    original->keyword = obj.keyword;
    original->short_descr = obj.short_descr;
    // Same normalization as the boot loader, so a save matches what the next boot loads.
    obj.description[RU] = format_longdescr(obj.description[RU]);
    original->description = obj.description;
    original->sound        = obj.sound;
    original->smell        = obj.smell;
    original->vnum         = obj.vnum;
    original->reset_num    = obj.reset_num;
    original->material     = obj.material;
    
    original->behavior     = obj.behavior;
    obj.behavior = 0; 
    original->item_type    = obj.item_type;
    original->extra_flags  = obj.extra_flags;
    original->wear_flags   = obj.wear_flags;
    original->level        = obj.level;
    original->condition    = obj.condition;
    original->gram_gender  = obj.gram_gender;
    original->xml_weight   = obj.xml_weight;
    original->xml_cost     = obj.xml_cost;
    memcpy(original->value, obj.value, sizeof(obj.value));
    original->limit        = obj.limit;

    original->behaviors.clear();
    original->behaviors.set(obj.behaviors);
    original->props.clear();
    JsonUtils::copy(original->props, obj.props);

    // Weight and cost come from the item models on the finished prototype (its
    // material, values, props); instances still at the old values follow.
    int oldWeight = original->weight, oldCost = original->cost;
    item_economy_proto(original);
    obj.weight = original->weight;
    obj.cost = original->cost;

    for(o = object_list; o; o = o->next)
        if(o->pIndexData == original) {
            if(o->weight == oldWeight)
                obj_set_weight(o, original->weight);

            if(o->cost == oldCost)
                o->cost = original->cost;
        }

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

// Defined before every other subcommand: the chain matches the last defined
// first, so 'c', 'ca' and the rest keep resolving to what they did before.
OEDIT(cases)
{
    return olc_cases_command(ch, argument, obj.short_descr,
                             olc_cases_gender_obj(obj.gram_gender), false);
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
    ptc(ch, "Wear:     [%s] {D(wear; ? wear_flags){x\n\r",
            web_flag_menu(showWeb, wear_flags, "wear", wear_flags.names(pObj->wear_flags)).c_str());
    ptc(ch, "Extra:    [%s] {D(? extra_flags){x\n\r", extra_flags.names(pObj->extra_flags).c_str());
    ptc(ch, "Material: [%s] {D(? material){x\n\r", pObj->material.c_str());
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
    if (pObj->xml_weight > 0)
        ptc(ch, "Weight:   [%5d] {D(set; model %d, 'weight 0' = model){x\n\r", pObj->xml_weight, item_weight(pObj));
    else
        ptc(ch, "Weight:   [%5d] {D(model){x\n\r", item_proto_weight(pObj));
    
    if (!pObj->behaviors.isSet(bhv_random_weapon)) {
        int autoCost = item_auto_cost(pObj);
        ptc(ch, "Cost:     [%5d] {D(set %d, model %d: the lower one counts, a scripted item with no stats keeps the set one){x\n\r",
            item_proto_cost(pObj), pObj->xml_cost, autoCost);
    }

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

        // Perma-affect grant (#2758): a real skill type means the item confers it.
        Skill *grantSkill = paf->type.getElement();
        if (grantSkill != 0 && grantSkill->getIndex() > 0)
            ptc(ch, "  {Cgrants %s{x", grantSkill->getName().c_str());

        ptc(ch, "\n\r");
        cnt++;
    }
    if (!pObj->affected.empty())
        stc("{D        ? apply_flags{x\r\n", ch);

    show_obj_values(ch, pObj);

    if (pObj->item_type == ITEM_KEY)
        ch->send_to(key_proto_locks(ch, pObj->vnum));

    if (is_furniture(pObj)) {
        int set = 0, total = 0;
        for (int i = 0; Item::furnitureMsgTags[i]; i++, total++)
            if (!Item::furnitureMsgProp(pObj, Item::furnitureMsgTags[i]).empty())
                set++;
        ptc(ch, "Сообщения мебели: %d из %d {D(msg){x\n\r", set, total);
    }

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

// Perma-affects engine (#2758 phase 2): make the item permanently grant a
// skill/affect to its wearer. Unlike addaffect this stamps the affect's TYPE, so
// isAffected/identify recognise it; an optional flag bit can ride along.
OEDIT(addgrant)
{
    OBJ_INDEX_DATA *pObj;
    Affect *pAf;
    const FlagTable *table = 0;
    bitstring_t bit = 0;
    DLString args = argument;
    DLString buf = args.getOneArgument();

    EDIT_OBJ(ch, pObj);

    if (buf.empty()) {
        stc("Syntax:  addgrant <skill> [table bit]\n\r", ch);
        stc("The item permanently grants the named skill/affect to its wearer.\n\r", ch);
        stc("Optionally also set a flag bit, e.g.:  addgrant sanctuary affect_flags sanctuary\n\r", ch);
        return false;
    }

    Skill *skill = skillManager->findExisting(buf);
    if (skill == 0) {
        ptc(ch, "Skill/affect '%s' not found.\r\n", buf.c_str());
        return false;
    }

    buf = args.getOneArgument();
    if (!buf.empty()) {
        table = FlagTableRegistry::getTable(buf);
        if (table == 0) {
            ptc(ch, "Unknown flag table '%s' (affect_flags, detect_flags, ...).\r\n", buf.c_str());
            return false;
        }
        bit = table->bitstring(args);
        if (bit == NO_FLAG) {
            ptc(ch, "Invalid flag, see 'olchelp %s'.\r\n", buf.c_str());
            return false;
        }
    }

    pAf = AffectManager::getThis()->getAffect();
    pAf->type.assign(skill->getIndex());
    pAf->duration = -1;
    pAf->location = APPLY_NONE;
    pAf->location.setTable(&apply_flags);
    pAf->modifier = 0;
    pAf->bitvector.setTable(table);
    pAf->bitvector.setValue(bit);

    pObj->affected.push_front(pAf);

    ptc(ch, "Grant added: the item now confers '%s'%s.\r\n",
        skill->getName().c_str(), bit != 0 ? " with a flag bit" : "");
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

// Furniture messages kept in props (Item::FURNITURE_PROPS), saved with the
// area. The tag list and checks live here, the write goes through 'prop'.
// Registered before 'material' so a bare 'm' still means material.
OEDIT(msg)
{
    OBJ_INDEX_DATA *pObj;
    DLString args = argument;
    DLString tagArg = args.getOneArgument();
    DLString text = args.stripWhiteSpace();
    const char *tag = 0;

    EDIT_OBJ(ch, pObj);

    if (!is_furniture(pObj)) {
        stc("Сообщения мебели бывают только у предметов типа furniture и fountain.\r\n", ch);
        return false;
    }

    if (tagArg.empty()) {
        ostringstream buf;
        buf << "Сообщения мебели {D(msg <тег> <текст>, msg <тег> clear){x" << endl
            << "В тексте %1$ -- этот предмет, %2$ -- персонаж, с падежом или формами: %2$^C1, %1$O4, %2$Gо|ел|ла. Поле Fenia с тем же именем на предмете или прототипе важнее." << endl;
        for (int i = 0; Item::furnitureMsgTags[i]; i++) {
            DLString msg = Item::furnitureMsgProp(pObj, Item::furnitureMsgTags[i]);
            buf << "  " << Item::furnitureMsgTags[i] << ": "
                << (msg.empty() ? DLString("(нет)") : msg + "{x") << endl;
        }
        ch->send_to(buf);
        return false;
    }

    // The full tag or the part after "msg", in any case: msgSitChar, sitchar.
    for (int i = 0; Item::furnitureMsgTags[i]; i++) {
        const char *t = Item::furnitureMsgTags[i];
        if (!strcasecmp(tagArg.c_str(), t) || !strcasecmp(tagArg.c_str(), t + 3)) {
            tag = t;
            break;
        }
    }

    if (!tag) {
        ptc(ch, "Нет такого тега: %s. Список тегов: msg\r\n", tagArg.c_str());
        return false;
    }

    if (text.empty()) {
        ptc(ch, "Использование: msg %s <текст> или msg %s clear\r\n", tag, tag);
        return false;
    }

    // The web editor pastes back through 'prop', which knows its own syntax.
    if (arg_is_web(text)) {
        ptc(ch, "Для веб-редактора: prop %s %s web\r\n", Item::FURNITURE_PROPS, tag);
        return false;
    }

    Json::Value &props = obj.props;
    if (!props.isNull() && !props.isObject()) {
        stc("Свойства (props) этого предмета не объект JSON, сообщение некуда сохранить.\r\n", ch);
        return false;
    }

    bool wasNull = props.isNull();
    if (props.isMember(Item::FURNITURE_PROPS) && !props[Item::FURNITURE_PROPS].isObject())
        props[Item::FURNITURE_PROPS] = Json::Value(Json::objectValue);

    bool clear = arg_is_clear(text);
    DLString error;
    if (!clear && !arg_is_paste(text) && !Item::furnitureMsgValid(text, error)) {
        ptc(ch, "Сообщение не сохранено: %s.\r\n", error.c_str());
        return false;
    }

    Json::Value old;
    if (props.isMember(Item::FURNITURE_PROPS) && props[Item::FURNITURE_PROPS].isObject()
        && props[Item::FURNITURE_PROPS].isMember(tag))
        old = props[Item::FURNITURE_PROPS][tag];

    DLString propArgs;
    propArgs << Item::FURNITURE_PROPS << " " << tag << " " << (clear ? DLString("del") : text);
    bool changed = editProps(obj.behaviors, props, propArgs, true);

    // A pasted text is only known after the paste: check it the same way.
    Json::Value &msgs = props[Item::FURNITURE_PROPS];
    if (changed && msgs.isObject() && msgs.isMember(tag)
        && (!msgs[tag].isString() || !Item::furnitureMsgValid(msgs[tag].asString(), error)))
    {
        if (old.isNull())
            msgs.removeMember(tag);
        else
            msgs[tag] = old;
        ptc(ch, "Сообщение не сохранено: %s.\r\n", error.empty() ? "это не строка" : error.c_str());
        changed = false;
    }

    if (msgs.isNull() || (msgs.isObject() && msgs.empty()))
        props.removeMember(Item::FURNITURE_PROPS);
    if (wasNull && props.empty())
        props = Json::Value();

    return changed;
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

// Both edit the area-file value: weight > 0 overrides the model (0 = the model),
// cost is the most the item may cost (the model only lowers it). The effective
// values follow on commit.
OEDIT(weight)
{
    return numberEdit(0, 10000, obj.xml_weight);
}

OEDIT(cost)
{
    return numberEdit(0, 1000000, obj.xml_cost);
}

// Flag the prototype for removal: XMLArea::init leaves ITEM_DELETED
// prototypes out of the area file on the next save.
OBJ_INDEX_DATA * OLCStateObject::markDeleted(PCharacter *ch, const char *argument)
{
    int value;

    if (!is_number(argument) || (value = atoi(argument)) <= 0) {
        stc("Syntax:  edit object delete <vnum>\n\r", ch);
        return 0;
    }

    AreaIndexData *pArea = get_vnum_area(value);
    if (!pArea) {
        stc("OEdit:  That vnum is not assigned an area.\n\r", ch);
        return 0;
    }

    if (!can_edit(ch, value)) {
        stc("OEdit:  Vnum in an area you cannot build in.\n\r", ch);
        return 0;
    }

    OBJ_INDEX_DATA *obji = get_obj_index(value);
    if (!obji) {
        ptc(ch, "Предмет %d не найден.\n\r", value);
        return 0;
    }

    pArea->changed = true;
    SET_BIT(obji->extra_flags, ITEM_DELETED);
    ptc(ch, "[%u] (%N1) помечен к удалению.\n\r", obji->vnum, obji->getShortDescr(LANG_DEFAULT));
    return obji;
}

OEDIT(create)
{
    int value = olc_create_vnum(ch, ch->in_room->areaIndex(), argument, OLC_NDX_OBJ);

    if (value < 0)
        return false;

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
    // Every choice as plain text for telnet and screen readers; web clients
    // also get them as a menu on the Wear line of 'show'.
    if (!*argument)
        ptc(ch, "Wear flags: %s\r\n", flag_table_all_names(wear_flags).c_str());

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
    return editor(argument, obj.material, ED_NO_NEWLINE);
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
        COPY_FENIA,
        COPY_ERROR
    } mode;
    
    if (arg_is(arg1, "desc"))
        mode = COPY_DESC;
    else if (arg_is(arg1, "param"))
        mode = COPY_PARAM;
    else if (arg_is(arg1, "behavior"))
        mode = COPY_BHV;
    else if (arg_is(arg1, "fenia"))
        mode = COPY_FENIA;
    else 
        mode = COPY_ERROR;
            
    if (mode == COPY_ERROR || !arg2.isNumber()) {
        ch->pecho("Syntax: \r\n"
                    "  copy param <vnum> -- copy affects, level, flags and other parameters from <vnum> obj index.\r\n"
                    "  copy behav <vnum> -- copy behaviors and props.\r\n"
                    "  copy fenia <vnum> -- copy Fenia triggers (one-trigger scenarios), live at once, no undo, in-game only.\r\n"
                    "  copy desc <vnum>  -- copy name, short, long and extra descriptions from <vnum> obj index.\r\n" );
        return false;
    }
    
    original = get_obj_index( arg2.toInt() );

    if (original == NULL) {
        ch->pecho("Object not found.\r\n");
        return false;
    }

    if (!OLCState::can_edit(ch, original->vnum)) {
        stc("У тебя недостаточно прав для редактирования предметов.\n\r", ch);
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
    case COPY_FENIA:
        if (original->vnum == obj.vnum) {
            ch->pecho("Это тот же vnum.");
            return false;
        }
        feniaTriggers->copyTriggers(ch, "obj", original->vnum, obj.vnum);
        return false;
    default:
        return false;
    }
    
    ch->pecho("All %s copied from vnum %d (%N1).",
                report.c_str( ),
                original->vnum, 
                original->getShortDescr(LANG_DEFAULT));
    return true;
}

// Where the prototype is reset, grouped by reset type: on the floor, inside a
// container, in a mob's inventory, worn by a mob, in a Fenia shop's stock.
// Mobs with the shopper behavior are marked: what they carry is for sale.
OEDIT(list)
{
    OBJ_INDEX_DATA *pObj;
    ostringstream floor, container, inventory, equip, stock;
    int cnt = 0;

    EDIT_OBJ(ch, pObj);

    for (auto &r: roomIndexMap) {
        RoomIndexData *pRoom = r.second;
        MOB_INDEX_DATA *lastMob = 0;
        // Built only for a matching reset: most rooms have none.
        auto inRoom = [&]() {
            return fmt(0, "in room [%s] {g%s{x",
                    web_cmd(ch, "redit $1", DLString(pRoom->vnum)).c_str(),
                    pRoom->name.get(LANG_DEFAULT).c_str());
        };

        for (auto &pReset: pRoom->resets) {
            switch (pReset->command) {
            case 'M': {
                lastMob = get_mob_index(pReset->arg1);
                if (!lastMob || !mob_index_is_shopper(lastMob))
                    break;

                // A Fenia shop keeper sells its props stock, no G reset needed.
                list<int> shopStock = mob_index_shop_stock(lastMob);
                if (std::find(shopStock.begin(), shopStock.end(), pObj->vnum) != shopStock.end()) {
                    stock << fmt(0, "  [%s] {g%N1{x %s\n\r",
                            web_cmd(ch, "medit $1", DLString(lastMob->vnum)).c_str(),
                            lastMob->getShortDescr(LANG_DEFAULT), inRoom().c_str());
                    cnt++;
                }
                break;
            }

            case 'O':
                if (reset_loads_obj(pReset, pObj->vnum)) {
                    floor << "  " << inRoom() << "\n\r";
                    cnt++;
                }
                break;

            case 'P':
                if (reset_loads_obj(pReset, pObj->vnum)) {
                    OBJ_INDEX_DATA *pContainer = get_obj_index(pReset->arg3);
                    container << fmt(0, "  [%s] {g%N1{x %s\n\r",
                            web_cmd(ch, "oedit $1", DLString(pReset->arg3)).c_str(),
                            pContainer ? pContainer->getShortDescr(LANG_DEFAULT) : "?",
                            inRoom().c_str());
                    cnt++;
                }
                break;

            case 'G':
            case 'E': {
                if (!reset_loads_obj(pReset, pObj->vnum))
                    break;

                DLString mob = "  [?] (no mob)";
                if (lastMob)
                    mob = fmt(0, "  [%s] {g%N1{x%s",
                            web_cmd(ch, "medit $1", DLString(lastMob->vnum)).c_str(),
                            lastMob->getShortDescr(LANG_DEFAULT),
                            mob_index_is_shopper(lastMob) ? " {Gshopkeeper{x" : "");

                if (pReset->command == 'G') {
                    inventory << mob << " " << inRoom() << "\n\r";
                } else {
                    Wearlocation *wearloc = wearlocationManager->find(pReset->arg3);
                    equip << mob << " on " << (wearloc ? wearloc->getName() : DLString("?"))
                          << " " << inRoom() << "\n\r";
                }
                cnt++;
                break;
            }
            }
        }
    }

    ostringstream buffer;
    buffer << fmt(0, "Resets for object [{W%d{x] ({g%N1{x):\n\r",
            pObj->vnum, pObj->getShortDescr(LANG_DEFAULT));

    struct { const char *title; ostringstream *lines; } groups[] = {
        { "On the floor (O)", &floor },
        { "Inside a container (P)", &container },
        { "In mob inventory (G)", &inventory },
        { "Worn by a mob (E)", &equip },
        { "In a shop's stock", &stock },
    };

    for (auto &g: groups)
        if (!g.lines->str().empty())
            buffer << "{W" << g.title << ":{x\n\r" << g.lines->str();

    buffer << fmt(0, "Total {W%d{x resets found.\n\r", cnt);

    page_to_char(buffer.str().c_str(), ch);
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

    if (arg_is_clear(argument)) {        
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
    // See MEDIT(done): a proposal for a short this run typed bare.
    OBJ_INDEX_DATA *original = get_obj_index(obj.vnum);
    DLString report;
    olc_cases_propose(obj.short_descr, original ? &original->short_descr : 0,
                      olc_cases_gender_obj(obj.gram_gender), false, report);

    commit();
    ch->send_to(report);
    olc_validate_print(ch, "obj", obj.vnum);
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
    } else if (arg_is_strict(arg1, "create")) {
        value = olc_create_vnum(ch, ch->in_room->areaIndex(), argument, OLC_NDX_OBJ);
        if (value < 0)
            return;

        OLCStateObject::Pointer oe(NEW, value);
        oe->attach(ch);
        return;
    } else if (arg_is_strict(arg1, "delete")) {
        OLCStateObject::markDeleted(ch, argument);
        return;

    } else if(arg_is_strict(arg1, "show")) {
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
    } else if (arg_is_strict(arg1, "load")) {
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

DLString OLCStateObject::getJournalKind( ) const
{
    return "obj";
}

int OLCStateObject::getJournalVnum( ) const
{
    return obj.vnum;
}


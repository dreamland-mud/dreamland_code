/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>

#include "medit.h"


#include "grammar_entities_impl.h"
#include <pcharacter.h>
#include <npcharacter.h>
#include "race.h"
#include <object.h>
#include <affect.h>
#include "room.h"
#include "skillgroup.h"
#include "string_utils.h"
#include "websocketrpc.h"
#include "comm.h"
#include "merc.h"
#include "interp.h"
#include "loadsave.h"
#include "act.h"
#include "json_utils_ext.h"

#include "olc.h"
#include "security.h"
#include "feniatriggers.h"

#include "def.h"

OLC_STATE(OLCStateMobile);

// Remove after 'medit convert' removal
GSN(faerie_fire);
GSN(infravision);
GSN(fly);
GSN(curse);
GSN(regeneration);
GSN(sneak);
GSN(pass_door);
GSN(protection_good);
GSN(slow);
GSN(swim);
GSN(sanctuary);
GSN(haste);
GSN(protection_evil);
GSN(stardust);
GSN(dark_shroud);
GSN(corruption);
GSN(invisibility);
GSN(improved_invis);
GSN(hide);
GSN(fade);
GSN(camouflage);
GSN(blindness);
GSN(poison);
GSN(sleep);
GSN(charm_person);
GSN(calm);
GSN(plague);
GSN(weaken);
GSN(berserk);

CLAN(none);

OLCStateMobile::OLCStateMobile( )
{
    /*fromXML will fill fields for us*/
}

OLCStateMobile::OLCStateMobile( MOB_INDEX_DATA *original )
{
    const char *c = 0;
    
    if(original->spec_fun.func)
        c = spec_name(original->spec_fun.func);
        
    if(!c)
        c = original->spec_fun.name.c_str( );
    
    if(c && *c)
        mob.spec_fun.name = c;
    else
        mob.spec_fun.name = "";

    if(original->behavior)
        mob.behavior         = XMLDocument::Pointer( NEW, **original->behavior );
    else
        mob.behavior         = 0;

    mob.vnum             = original->vnum;
    mob.area             = original->area;

    copyParameters( original );
    copyDescriptions( original );
    copyBehaviors(original);
}

void OLCStateMobile::copyBehaviors(MOB_INDEX_DATA *original)
{
    mob.behaviors.clear();
    mob.behaviors.set(original->behaviors);
    mob.props.clear();
    JsonUtils::copy(mob.props, original->props);
}

void OLCStateMobile::copyDescriptions( MOB_INDEX_DATA *original )
{
    mob.keyword = original->keyword;
    mob.short_descr = original->short_descr;
    mob.long_descr = original->long_descr;
    mob.description = original->description;
    mob.material         = original->material;
    mob.smell            = original->smell;
}

void OLCStateMobile::copyParameters( MOB_INDEX_DATA *original )
{
    mob.group            = original->group;
    mob.act              = original->act;
    mob.affected_by      = original->affected_by;
    mob.detection        = original->detection;
    mob.alignment        = original->alignment;
    mob.level            = original->level;
    mob.hitroll          = original->hitroll;
    memcpy(mob.hit, original->hit, sizeof(mob.hit));
    memcpy(mob.mana, original->mana, sizeof(mob.mana));
    memcpy(mob.damage, original->damage, sizeof(mob.damage));
    memcpy(mob.ac, original->ac, sizeof(mob.ac));
    mob.dam_type         = original->dam_type;
    mob.off_flags        = original->off_flags;
    mob.imm_flags        = original->imm_flags;
    mob.res_flags        = original->res_flags;
    mob.vuln_flags       = original->vuln_flags;
    mob.start_pos        = original->start_pos;
    mob.default_pos      = original->default_pos;
    mob.sex              = original->sex;
    mob.gram_number      = original->gram_number;
    mob.race             = original->race;
    mob.wealth           = original->wealth;
    mob.form             = original->form;
    mob.parts            = original->parts;
    mob.size             = original->size;
    mob.practicer.clear( );
    mob.practicer.set( original->practicer );
    mob.religion.clear();
    mob.religion.set(original->religion);
    mob.affects.clear();
    mob.affects.set(original->affects);
    mob.clan = original->clan;
}

OLCStateMobile::OLCStateMobile( int vnum )
{
    mob.vnum = vnum;
    mob.area = get_vnum_area(vnum);
}

OLCStateMobile::~OLCStateMobile( )
{
}

void OLCStateMobile::commit()
{
    Character *wch;
    MOB_INDEX_DATA *original;

    original = get_mob_index(mob.vnum);

    if(!original) {
        int iHash;
        
        original = new mob_index_data;

        original->vnum = mob.vnum;
        original->area = mob.area;

        mob.act |= ACT_IS_NPC;
        iHash = (int) mob.vnum % MAX_KEY_HASH;
        original->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = original;
    }


    for(wch = char_list; wch; wch = wch->next) {
        NPCharacter *victim = wch->getNPC();

        if (!victim)
            continue;

        if(victim->pIndexData == original) {
            if(victim->spec_fun.func == original->spec_fun.func) {
                victim->spec_fun.name = mob.spec_fun.name;
                victim->spec_fun.func = spec_lookup( mob.spec_fun.name.c_str() );
            }
            
            if(victim->group == original->group)
                victim->group = mob.group;

            if(String::toString(victim->getKeyword()) == String::toString(original->keyword))
                victim->setKeyword(mob.keyword);

            if(victim->act == original->act)
                victim->act = mob.act;
            
            if(victim->affected_by == original->affected_by)
                victim->affected_by = mob.affected_by;
            
            if(victim->detection == original->detection)
                victim->detection = mob.detection;
            
            if(victim->alignment == original->alignment)
                victim->alignment = mob.alignment;
            
            if(victim->getRealLevel() == original->level)
                victim->setLevel(mob.level);

            if(victim->hitroll == original->hitroll)
                victim->hitroll = mob.hitroll;
            
            if(victim->material == original->material)
                victim->material = mob.material;

/* - rt values. not to be fixed
            victim->hit              = mob.hit;
            victim->mana             = mob.mana;
            victim->damage           = mob.damage;
            victim->ac               = mob.ac;
*/
            if(victim->dam_type == original->dam_type)
                victim->dam_type = mob.dam_type;
            
            if(victim->off_flags == original->off_flags)
                victim->off_flags = mob.off_flags;

            if(victim->imm_flags == original->imm_flags)
                victim->imm_flags = mob.imm_flags;
            
            if(victim->res_flags == original->res_flags)
                victim->res_flags = mob.res_flags;
            
            if(victim->vuln_flags == original->vuln_flags)
                victim->vuln_flags = mob.vuln_flags;
            
            if(victim->start_pos == original->start_pos)
                victim->start_pos = mob.start_pos;
            
            if(victim->default_pos == original->default_pos)
                victim->default_pos = mob.default_pos;
            
            if(victim->getSex() == original->sex)
                victim->setSex( mob.sex );
            
            if(victim->getRace()->getName( ) == original->race)
                victim->setRace( mob.race );

            if(victim->form == original->form)
                victim->form             = mob.form;
            
            if(victim->parts == original->parts)
                victim->parts = mob.parts;

/*
  don't touch, mob size value can be affected by worn items
            if(victim->size == original->getSize())
                victim->size  = mob.size == NO_FLAG ? victim->getRace()->getSize() : mob.size;
*/                
        }
    }

    original->keyword = mob.keyword;
    original->short_descr = mob.short_descr;
    original->long_descr = mob.long_descr;
    original->description = mob.description;
    original->spec_fun.name    = mob.spec_fun.name;
    original->spec_fun.func    = spec_lookup( mob.spec_fun.name.c_str() );
    original->vnum             = mob.vnum;
    original->group            = mob.group;
    original->smell            = mob.smell;
    mob.smell.clear( );
    original->act              = mob.act;
    original->affected_by      = mob.affected_by;
    original->detection        = mob.detection;
    original->alignment        = mob.alignment;
    original->level            = mob.level;
    original->hitroll          = mob.hitroll;
    memcpy(original->hit, mob.hit, sizeof(mob.hit));
    memcpy(original->mana, mob.mana, sizeof(mob.mana));
    memcpy(original->damage, mob.damage, sizeof(mob.damage));
    memcpy(original->ac, mob.ac, sizeof(mob.ac));
    original->dam_type         = mob.dam_type;
    original->off_flags        = mob.off_flags;
    original->imm_flags        = mob.imm_flags;
    original->res_flags        = mob.res_flags;
    original->vuln_flags       = mob.vuln_flags;
    original->start_pos        = mob.start_pos;
    original->default_pos      = mob.default_pos;
    original->sex              = mob.sex;
    original->gram_number      = mob.gram_number;
    original->race             = mob.race;
    original->wealth           = mob.wealth;
    original->form             = mob.form;
    original->parts            = mob.parts;
    original->size             = mob.size;
    original->material         = mob.material;
    original->behavior         = mob.behavior;
    mob.behavior = 0;    
    original->practicer.clear( );
    original->practicer.set( mob.practicer );
    original->religion.clear();
    original->religion.set(mob.religion);
    original->affects.clear();
    original->affects.set(mob.affects);
    original->clan = mob.clan;

    for(wch = char_list; wch; wch = wch->next) {
        NPCharacter *victim = wch->getNPC();

        if (victim && victim->pIndexData == original) 
            victim->updateCachedNouns( );
    }

    original->behaviors.clear();
    original->behaviors.set(mob.behaviors);
    original->props.clear();
    JsonUtils::copy(original->props, mob.props);
}

void OLCStateMobile::statePrompt(Descriptor *d)
{
    d->send( "Editing mob> " );
}

// Mobile Editor Functions.
MEDIT(show)
{
    bool showWeb = !arg_is_strict(argument, "noweb");
    Race *race = raceManager->findExisting(mob.race);

    ptc(ch, "{CVnum: [{Y%u{C]  Area: [{Y%5d{C] {G%s{x\n\r",
        mob.vnum,
        !mob.area ? -1 : mob.area->vnum,
        !mob.area ? "No Area" : mob.area->getName().c_str());

    ptc(ch, "{GName EN:{x  [{W%s{w]{x %s\n\r", mob.keyword.get(EN).c_str(), web_edit_button(showWeb, ch, "name", "web").c_str());
    ptc(ch, "{GName UA:{x  [{W%s{w]{x %s\n\r", mob.keyword.get(UA).c_str(), web_edit_button(showWeb, ch, "uaname", "web").c_str());
    ptc(ch, "{GName RU:{x  [{W%s{w]{x %s\n\r", mob.keyword.get(RU).c_str(), web_edit_button(showWeb, ch, "runame", "web").c_str());

    ptc(ch, "{GShort EN:{x [{W%s{w]{x %s\n\r", mob.short_descr.get(EN).c_str(), web_edit_button(showWeb, ch, "short", "web").c_str());
    ptc(ch, "{GShort UA:{x [{W%s{w]{x %s\n\r", mob.short_descr.get(UA).c_str(), web_edit_button(showWeb, ch, "uashort", "web").c_str());
    ptc(ch, "{GShort RU:{x [{W%s{w]{x %s\n\r", mob.short_descr.get(RU).c_str(), web_edit_button(showWeb, ch, "rushort", "web").c_str());

    ptc(ch, "{GLong EN:{x  [{W%s{w]{x %s\n\r", String::stripEOL(mob.long_descr.get(EN)).c_str(), web_edit_button(showWeb, ch, "long", "web").c_str());        
    ptc(ch, "{GLong UA:{x  [{W%s{w]{x %s\n\r", String::stripEOL(mob.long_descr.get(UA)).c_str(), web_edit_button(showWeb, ch, "ualong", "web").c_str());        
    ptc(ch, "{GLong RU:{x  [{W%s{w]{x %s\n\r", String::stripEOL(mob.long_descr.get(RU)).c_str(), web_edit_button(showWeb, ch, "rulong", "web").c_str());        
    
    ptc(ch, "{CLevel{x:       [{W%3d{x]\n\r", mob.level);

    ptc(ch, "{CRace{x:        [{W%s{x] {D(? race){x Sex: [{W%s{x] {D(? sex_table){x Number: [{W%s{x]\n\r",
        mob.race.c_str(), 
        sex_table.name( mob.sex ).c_str( ),
        mob.gram_number == Grammar::Number::PLURAL ? "plural" : "singular");

    ptc(ch, "{CHit dice:{x    [{W%6dd%-5d+%6d{x]\n\r",
        mob.hit[DICE_NUMBER],
        mob.hit[DICE_TYPE],
        mob.hit[DICE_BONUS]);

    ptc(ch, "{CDamage dice:{x [{W%6dd%-5d+%6d{x]\n\r",
        mob.damage[DICE_NUMBER],
        mob.damage[DICE_TYPE],
        mob.damage[DICE_BONUS]);

    ptc(ch, "{CMana dice:{x   [{W%6dd%-5d+%6d{x]\n\r",
        mob.mana[DICE_NUMBER],
        mob.mana[DICE_TYPE],
        mob.mana[DICE_BONUS]);

    ptc(ch, "{CWealth:{x      [{W%10d{x]  {CAlign{x: [{W%5d{x]\n\r",
        mob.wealth, mob.alignment);

    ptc(ch, "{CHitroll:{x     [{W%5d{x]  {CDamage{x: [{W%10s{x] {D(? weapon_flags){x\n\r",
        mob.hitroll, weapon_flags.name(mob.dam_type).c_str( ));

    ptc(ch, "{CArmor{x:       [{Wpierce: %d  bash: %d  slash: %d  magic: %d{x] {D(? ac_type){x\n\r",
        mob.ac[AC_PIERCE], mob.ac[AC_BASH],
        mob.ac[AC_SLASH], mob.ac[AC_EXOTIC]);

    ptc(ch, "{CAct{x:         [{W%s{x] {D(? act_flags){x\n\r", act_flags.names(mob.act).c_str());

    ptc(ch, "{CAff:{x         [{W%s{x] {D(oaff){x\n\r", affect_flags.names(mob.affected_by).c_str());
    ptc(ch, "{CAffects:{x     [{W%s{x] {D(aff){x\n\r", mob.affects.toString().c_str());
    ptc(ch, "{CDet:{x         [{W%s{x] {D(? detect_flags){x\n\r", detect_flags.names(mob.detection).c_str());

    ptc(ch, "{CPosition:{x    starting [{W%s{x]  default [{W%s{x] {D(? position_table){x\n\r",
        position_table.name(mob.start_pos).c_str(),
        position_table.name(mob.default_pos).c_str());

    ptc(ch, "{CImm:{x         [{W%s{x] {D(? imm_flags){x\n\r", imm_flags.names(mob.imm_flags).c_str());
    ptc(ch, "{CRes:{x         [{W%s{x] {D(? res_flags){x\n\r", res_flags.names(mob.res_flags).c_str());
    ptc(ch, "{CVuln:{x        [{W%s{x] {D(? vuln_flags){x\n\r", vuln_flags.names(mob.vuln_flags).c_str());
    ptc(ch, "{COff:{x         [{W%s{x] {D(? off_flags){x\n\r", off_flags.names(mob.off_flags).c_str());
    ptc(ch, "{CSize:{x        [{W%s{x] (расовый {g%s{x) {D(? size_table){x\n\r", 
        size_table.name(mob.size).c_str(), 
        race ? race->getSize().name().c_str() : "-");

    ptc(ch, "{CMaterial:{x    [{W%s{x] {D(? material){x\n\r", mob.material.c_str());
    ptc(ch, "{CForm:{x        [{W%s{x] {D(? form_flags){x\n\r", form_flags.names(mob.form).c_str());
    ptc(ch, "{CParts:{x       [{W%s{x] {D(? part_flags){x\n\r", part_flags.names(mob.parts).c_str());

    if (!mob.spec_fun.name.empty())
        ptc(ch, "{CSpec fun:{x    [{W%s{x] {D(? spec){x\n\r", mob.spec_fun.name.c_str());
    ptc(ch, "{CGroup:{x       [{W%d{x]\n\r", mob.group);
    ptc(ch, "{CPracticer:{x   [{W%s{x] {D(? groups){x\n\r", mob.practicer.toString( ).c_str( ));
    ptc(ch, "{CReligion:{x    [{W%s{x] {D(reledit list){x\n\r", mob.religion.toString().c_str());
    if (mob.clan != clan_none)
        ptc(ch, "{CClan:{x        [{W%s{x] {D(? clan){x\n\r", mob.clan->getName().c_str());

    ptc(ch, "{GSmell:{x       [{W%s{x] %s [{W%s{x] %s [{W%s{x] %s\n\r", 
          String::stripEOL(mob.smell.get(EN)).c_str(), web_edit_button(showWeb, ch, "smell", "web").c_str(),   
          String::stripEOL(mob.smell.get(UA)).c_str(), web_edit_button(showWeb, ch, "uasmell", "web").c_str(),   
          String::stripEOL(mob.smell.get(RU)).c_str(), web_edit_button(showWeb, ch, "rusmell", "web").c_str());   

    const size_t descSize = 100;
    DLString entext = String::ellipsis(mob.description.get(EN), descSize);
    DLString uatext = String::ellipsis(mob.description.get(UA), descSize);
    DLString rutext = String::ellipsis(mob.description.get(RU), descSize);
    ptc(ch, "{GDesc EN:{x %s\n%s\n", web_edit_button(showWeb, ch, "desc", "web").c_str(), entext.c_str());
    ptc(ch, "{GDesc UA:{x %s\n%s\n", web_edit_button(showWeb, ch, "uadesc", "web").c_str(), uatext.c_str());
    ptc(ch, "{GDesc RU:{x %s\n%s\n", web_edit_button(showWeb, ch, "rudesc", "web").c_str(), rutext.c_str());

    if (mob.behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            mob.behavior->save( ostr );
            ptc(ch, "Legacy behavior: {D(oldbehavior{x)\r\n%s", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Legacy behavior is BUGGY.\r\n");
        }
    }

    show_behaviors(ch, mob.behaviors, mob.props);

    MOB_INDEX_DATA *original = get_mob_index(mob.vnum);
    feniaTriggers->showTriggers(ch, original ? get_wrapper(original->wrapper) : 0, "mob");    
    return false;
}

MEDIT(behaviors)
{
    return editBehaviors(mob.behaviors, mob.props);
}

MEDIT(fenia)
{
    feniaTriggers->openEditor(ch, mob, argument);
    return false;
}

MEDIT(props)
{
    return editProps(mob.behaviors, mob.props, argument);
}

MEDIT(create)
{
    AreaIndexData *pArea;
    int value;

    value = atoi(argument);
    if (argument[0] == '\0' || value == 0) {
        stc("Syntax:  medit create [vnum]\n\r", ch);
        return false;
    }

    pArea = get_vnum_area(value);

    if (!pArea) {
        stc("OLCStateMobile:  That vnum is not assigned an area.\n\r", ch);
        return false;
    }

    if (!can_edit( ch, value )) {
        stc("OLCStateMobile:  Vnum in an area you cannot build in.\n\r", ch);
        return false;
    }

    if (get_mob_index(value)) {
        stc("OLCStateMobile:  Mobile vnum already exists.\n\r", ch);
        return false;
    }


    Pointer me(NEW, value);
    me->attach(ch);

    stc("Mobile Created.\n\r", ch);
    return true;
}

static void xml_node_set( Character *ch, XMLNode::Pointer root, const DLString &nodeName, const DLString &nodeValue )
{
    XMLNode::Pointer node;
    
    if (!root) {
        stc("Корневой элемент документа пуст!\n\r", ch);
        return;
    }

    node = root->selectSingleNode( nodeName );
    if (node) {
        if (node->getFirstNode( )) {
            node->getFirstNode( )->setType( XMLNode::XML_TEXT );
            node->getFirstNode( )->setName( nodeValue );
        } else {
            stc("Ошибка: неправильный элемент.\n\r", ch);
            return;
        }
    }
    else {
        node.construct( );
        node->setName( nodeName );
        root->appendChild( node );

        XMLNode::Pointer child( NEW );
        child->setType( XMLNode::XML_TEXT );
        child->setCData( nodeValue );
        node->appendChild( child );
    }

    ptc(ch, "Полю {W%s{x присвоено значение {G%s{x.\n\r", nodeName.c_str( ), nodeValue.c_str( ));
}

MEDIT(shop)
{
    char cmd[MAX_INPUT_LENGTH];
    int value = -1;
    bitstring_t itypes = NO_FLAG;
    DLString nodeValue;

    argument = one_argument( argument, cmd );
    
    if (!cmd[0]) {
        stc("Используйте 'shop help' для справки.\r\n", ch);
        return false;
    }

    if (!mob.behavior) {
        stc("Поведение продавца не задано, используйте 'oldbehavior shopper'.\r\n", ch);
        return false;
    }

    if (is_number( argument )) {
        value = atoi( argument );
        nodeValue = value;
    }
    else {
        itypes = item_table.bitstring( argument, false );
        nodeValue = item_table.names( itypes );
    }
    
    XMLNode::Pointer root = mob.behavior->getDocumentElement( );

    if (!str_prefix( cmd, "sellprofit" )) {
        if (value != -1) {
            xml_node_set( ch, root, "profitSell", nodeValue );
            return true;
        }
    }
    else if (!str_prefix( cmd, "buyprofit" )) {
        if (value != -1) {
            xml_node_set( ch, root, "profitBuy", nodeValue );
            return true;
        }
    }
    else if (!str_prefix( cmd, "openhour" )) {
        if (value >= 0 && value < 24) {
            xml_node_set( ch, root, "openHour", nodeValue );
            return true;
        }
    }
    else if (!str_prefix( cmd, "closehour" )) {
        if (value >= 0 && value < 24) {
            xml_node_set( ch, root, "closeHour", nodeValue );
            return true;
        }
    }
    else if (!str_prefix( cmd, "buys" )) {
        if (itypes != NO_FLAG) {
            xml_node_set( ch, root, "buys", nodeValue );
            return true;
        }
    }
    else if (!str_prefix( cmd, "repairs" )) {
        if (itypes != NO_FLAG) {
            xml_node_set( ch, root, "repairs", nodeValue );
            return true;
        }
    }

    stc("shop sellprofit <number>  - наценка от продажи, в процентах\r\n"
        "shop buyprofit  <number>  - наценка от покупки, в процентах\r\n"
        "shop openhour   <number>  - в котором часу открывается (0..23)\r\n"
        "shop closehour  <number>  - в котором часу закрывается (0..23)\r\n"
        "shop buys <item types>    - какие типы предметов покупает \r\n"
        "shop repairs <item types> - какие типы предметов ремонтирует \r\n",
        ch);
    return false;
}

MEDIT(where)
{
    ch->pecho("%N1 находится в: ", mob.getShortDescr(LANG_DEFAULT));

    for (Character *wch = char_list; wch; wch = wch->next) {
        if (!wch->is_npc( ))
            continue;

        if (wch->getNPC( )->pIndexData->vnum != mob.vnum)
            continue;

        ptc(ch, "[%5d]    %-30s (%s)\r\n", 
                wch->in_room->vnum,
                wch->in_room->getName(), 
                wch->in_room->areaName().c_str());
    }

    return true;
}

MEDIT(spec)
{
    if (argument[0] == '\0') {
        stc("Syntax:  spec [special function]\n\r", ch);
        return false;
    }


    if (arg_is_clear(argument)) {
        mob.spec_fun.name = "";

        stc("Spec removed.\n\r", ch);
        return true;
    }

    SPEC_FUN *sp = spec_lookup(argument);
    if (sp) {
        const char *c = spec_name(sp);
        if(c) {
            mob.spec_fun.name = c;
            stc("Spec set.\n\r", ch);
            return true;
        }
    }

    stc("OLCStateMobile: No such special function.\n\r", ch);
    return false;
}

MEDIT(damtype)
{
    return flagValueEdit(weapon_flags, mob.dam_type);
}


MEDIT(align)
{
    return numberEdit(-1000, 1000, mob.alignment);
}

MEDIT(level)
{
    return numberEdit(1, 120, mob.level);
}

MEDIT(desc)
{
    return editor(argument, mob.description[EN]);
}

MEDIT(uadesc)
{
    return editor(argument, mob.description[UA]);
}

MEDIT(rudesc)
{
    return editor(argument, mob.description[RU]);
}

MEDIT(smell)
{
    return editor(argument, mob.smell[EN]);
}

MEDIT(uasmell)
{
    return editor(argument, mob.smell[UA]);
}

MEDIT(rusmell)
{
    return editor(argument, mob.smell[RU]);
}

MEDIT(oldbehavior)
{
    DLString type;
    DLString arg = argument;

    if (arg.empty()) {
        if(!xmledit(mob.behavior))
            return false;

        stc("Поведение установлено.\r\n", ch);
        return true;
    }

    if (mob.behavior) {
        if (arg_is_clear(arg)) {
            mob.behavior.clear( );
            stc("Поведение очищено.\r\n", ch);
            return true;
        }

        stc("Поведение уже задано, используйте 'oldbehavior' для редактирования или 'oldbehavior clear' для очистки.\r\n", ch);
        return false;
    }

    if (arg == "shopper") {
        static const DLString shopperType( "ShopTrader" );
        type = shopperType;
    }
    else if (arg == "pet") {
        static const DLString petType( "Pet" );
        type = petType;
    }
    else if (arg == "leveladaptivepet") {
        static const DLString petType( "LevelAdaptivePet" );
        type = petType;
    }
    else if (arg == "trainer") {
        static const DLString trainerType( "Trainer" );
        type = trainerType;
    }
    else if (arg == "savedcreature") {
        static const DLString savedCreatureType( "SavedCreature" );
        type = savedCreatureType;
    }
    else { 
        stc("Допустимые значения поведения: savedcreature, shopper, pet, leveladaptivepet, trainer.\r\n", ch);
        return false;
    }

    mob.behavior = XMLDocument::Pointer( NEW );

    XMLNode::Pointer node( NEW );
    node->insertAttribute( XMLNode::ATTRIBUTE_TYPE, type );
    node->setName( "behavior" );
    mob.behavior->appendChild( node );

    ptc(ch, "Поведение {G%s{x установлено.\r\n", type.c_str( ));
    return true;
}

MEDIT(clan)
{  
    return globalReferenceEdit<ClanManager, Clan>(mob.clan);
}

MEDIT(long)
{
    return editor(argument, mob.long_descr[EN], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_ADD_NEWLINE));
}

MEDIT(ualong)
{
    return editor(argument, mob.long_descr[UA], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_ADD_NEWLINE));
}

MEDIT(rulong)
{
    return editor(argument, mob.long_descr[RU], (editor_flags)(ED_UPPER_FIRST_CHAR|ED_ADD_NEWLINE));
}

MEDIT(short)
{
    return editor(argument, mob.short_descr[EN], ED_NO_NEWLINE);
}

MEDIT(uashort)
{
    return editor(argument, mob.short_descr[UA], ED_NO_NEWLINE);
}

MEDIT(rushort)
{
    return editor(argument, mob.short_descr[RU], ED_NO_NEWLINE);
}

MEDIT(name)
{
    return editor(argument, mob.keyword[EN], ED_NO_NEWLINE);
}

MEDIT(uaname)
{
    return editor(argument, mob.keyword[UA], ED_NO_NEWLINE);
}

MEDIT(runame)
{
    return editor(argument, mob.keyword[RU], ED_NO_NEWLINE);
}


MEDIT(sex)
{
    return flagValueEdit(sex_table, mob.sex);
}

MEDIT(number)
{
    if (argument[0] == '\0') {
        stc("Syntax:  number s|p\n\r", ch);
        return false;
    }

    mob.gram_number = Grammar::Number(argument);

    ptc(ch, "Grammatical number set to '%s'.\n\r", mob.gram_number.toString());
    return true;
}

MEDIT(act)                
{
    if (flagBitsEdit(act_flags, mob.act)) {
        SET_BIT(mob.act, ACT_IS_NPC);
        return true;
    }
    return false;
}

MEDIT(oaff)
{                                

    return flagBitsEdit(affect_flags, mob.affected_by);
}

MEDIT(detection)
{
    return flagBitsEdit(detect_flags, mob.detection);
}

MEDIT(ac)
{
    char arg[MAX_INPUT_LENGTH];
    int pierce, bash, slash, exotic;

    do {                        /* So that I can use break and send the syntax in one place */
        if (argument[0] == '\0')
            break;

        argument = one_argument(argument, arg);

        if (!is_number(arg))
            break;
        pierce = atoi(arg);
        argument = one_argument(argument, arg);

        if (arg[0] != '\0') {
            if (!is_number(arg))
                break;
            bash = atoi(arg);
            argument = one_argument(argument, arg);
        }
        else
            bash = mob.ac[AC_BASH];

        if (arg[0] != '\0') {
            if (!is_number(arg))
                break;
            slash = atoi(arg);
            argument = one_argument(argument, arg);
        }
        else
            slash = mob.ac[AC_SLASH];

        if (arg[0] != '\0') {
            if (!is_number(arg))
                break;
            exotic = atoi(arg);
        }
        else
            exotic = mob.ac[AC_EXOTIC];

        mob.ac[AC_PIERCE] = pierce;
        mob.ac[AC_BASH] = bash;
        mob.ac[AC_SLASH] = slash;
        mob.ac[AC_EXOTIC] = exotic;

        stc("Ac set.\n\r", ch);
        return true;
    } while (false);                /* Just do it once.. */

    stc("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
        "help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
    return false;
}


MEDIT(form)
{
    return flagBitsEdit(form_flags, mob.form);
}

MEDIT(part)
{
    return flagBitsEdit(part_flags, mob.parts);
}

MEDIT(imm)
{
    return flagBitsEdit(imm_flags, mob.imm_flags);
}

MEDIT(res)
{
    return flagBitsEdit(res_flags, mob.res_flags);
}

MEDIT(vuln)
{
    return flagBitsEdit(vuln_flags, mob.vuln_flags);
}

MEDIT(material)
{
    return editor(argument, mob.material, ED_NO_NEWLINE);
}

MEDIT(off)
{
    return flagBitsEdit(off_flags, mob.off_flags);
}

MEDIT(size)
{
    return flagValueEdit(size_table, mob.size);
}

MEDIT(hitdice)
{
    return diceEdit(mob.hit);
}

MEDIT(manadice)
{
    return diceEdit(mob.mana);
}

MEDIT(damdice)
{
    return diceEdit(mob.damage);
}

MEDIT(race)
{
    Race *race;

    if (*argument) {
        race = raceManager->findUnstrict(argument);

        if(race) {
/*            
            Race *old_race = raceManager->find( mob.race );
            mob.off_flags &= ~old_race->getOff( );
            mob.imm_flags &= ~old_race->getImm( );
            mob.res_flags &= ~old_race->getRes( );
            mob.vuln_flags &= ~old_race->getVuln( );
            mob.form &= ~old_race->getForm( );
            mob.parts &= ~old_race->getParts( );
*/            

            mob.race = race->getName();

/*
            mob.off_flags |= race->getOff( );
            mob.imm_flags |= race->getImm( );
            mob.res_flags |= race->getRes( );
            mob.vuln_flags |= race->getVuln( );
            mob.form |= race->getForm( );
            mob.parts |= race->getParts( );
*/            
            stc("Race set.\n\r", ch);
            return true;
        } else {
            stc("Wrong race name.\n\t", ch);
        }
    }
    
    stc("Available races are:", ch);
    auto raceNames = raceManager->nameList();
    ch->pecho(print_columns(raceNames, 16, 3));
    return false;
}

MEDIT(position)
{
    char arg[MAX_INPUT_LENGTH];
    int value;

    argument = one_argument(argument, arg);

    switch (arg[0]) {
    default:
        break;

    case 'S':
    case 's':
        if (str_prefix(arg, "start"))
            break;

        if ((value = position_table.value( argument )) == NO_FLAG)
            break;

        mob.start_pos = (int) value;
        stc("Start position set.\n\r", ch);
        return true;

    case 'D':
    case 'd':
        if (str_prefix(arg, "default"))
            break;

        if ((value = position_table.value( argument )) == NO_FLAG)
            break;

        mob.default_pos = (int) value;
        stc("Default position set.\n\r", ch);
        return true;
    }

    stc("Syntax:  position [start/default] [position]\n\r"
        "Type '? position' for a list of positions.\n\r", ch);
    return false;
}

MEDIT(wealth)
{
    return numberEdit(0, 1000000, mob.wealth);
}

MEDIT(group)
{
    return numberEdit(0, 100000, mob.group);
}

MEDIT(practicer)
{
    return globalBitvectorEdit<SkillGroup>(mob.practicer);
}

MEDIT(affects)
{
    return globalBitvectorEdit<Skill>(mob.affects);
}

MEDIT(religion)
{
    return globalBitvectorEdit<Religion>(mob.religion);
}

MEDIT(hitroll)
{
    return numberEdit(0, 10000, mob.hitroll);
}

MEDIT(list)
{
    int cnt;
    RoomIndexData *pRoom;
    ostringstream buffer;
    
    buffer << fmt(0, "Resets for mobile [{W%d{x] ({g%N1{x):\n\r",
            mob.vnum, 
            mob.getShortDescr(LANG_DEFAULT));
    
    cnt = 0;
    for (auto &r: roomIndexMap) {
        pRoom = r.second;
        for(auto &pReset: pRoom->resets)
            switch(pReset->command) {
                case 'M':
                    if(pReset->arg1 == mob.vnum) {
                        buffer << fmt(0, "{G%c{x in room [{W%d{x] ({g%s{x)\n\r",
                                pReset->command, pRoom->vnum, pRoom->name.get(LANG_DEFAULT).c_str());
                        cnt++;
                    }
            }
    }

    buffer << fmt(0, "Total {W%d{x resets found.\n\r", cnt);
    
    page_to_char(buffer.str( ).c_str( ), ch);

    return false;
}

MEDIT(commands)
{
    do_commands(ch);
    return false;
}

MEDIT(done) 
{
    commit();
    detach(ch);
    return true;
}

MEDIT(cancel)
{
    detach(ch);
    return false;
}

MEDIT(dump)
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}

/*
 * XXX: theoretically, can be used to peek stats of mobiles 
 * that you have no right to edit
 */
MEDIT(copy)
{
    MOB_INDEX_DATA *original;
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
                    "  copy param <vnum> -- copy race, hit, damage and other parameters from <vnum> mob index.\r\n"
                    "  copy behav <vnum> -- copy behaviors and props.\r\n"
                    "  copy desc <vnum>  -- copy name, description, short and long description from <vnum> mob index.\r\n" );
        return false;
    }
    
    original = get_mob_index( arg2.toInt() );

    if (original == NULL) {
        ch->pecho("Mobile not found.\r\n");
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

CMD(medit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online mob editor.")
{
    NPCharacter *mob;
    MOB_INDEX_DATA *pMob;
    AreaIndexData *pArea;
    int value;
    char arg1[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg1);

    if (is_number(arg1)) {
        value = atoi(arg1);
        if (!(pMob = get_mob_index(value))) {
            stc("OLCStateMobile:  That vnum does not exist.\n\r", ch);
            return;
        }

        if(!pMob->area) {
            stc("this mobile has no area!!!!!\n\r", ch);
            return;
        }

        if (!OLCState::can_edit( ch, pMob->vnum )) {
            stc("У тебя недостаточно прав для редактирования монстров.\n\r", ch);
            return;
        }

        OLCStateMobile::Pointer me(NEW, pMob);
        me->attach(ch);
        me->findCommand(ch, "show")->entryPoint(ch, "");
        return;
    }
    else if (arg_is_strict(arg1, "create")) {
        if (arg_is_strict(argument, "next")) {
            value = next_mob_index(ch, ch->in_room->pIndexData);
            if (value < 0) {
                ch->pecho("Все внумы в этой зоне уже заняты!");
                return;
            }
        }
        else
            value = atoi(argument);

        if (arg1[0] == '\0' || value <= 0) {
            stc("Syntax:  edit mobile create <vnum>|next\n\r", ch);
            return;
        }

        pArea = OLCState::get_vnum_area(value);

        if (!pArea) {
            stc("MEdit:  That vnum is not assigned an area.\n\r", ch);
            return;
        }

        if (!OLCState::can_edit( ch, value )) {
            stc("У тебя недостаточно прав для редактирования монстров.\n\r", ch);
            return;
        }
        
        if (get_mob_index(value)) {
            stc("OLCStateMobile:  Mobile vnum already exists.\n\r", ch);
            return;
        }

        OLCStateMobile::Pointer me(NEW, value);
        me->attach(ch);
        return;
    } else if(arg_is_strict(arg1, "show")) {
        if(!*argument || !is_number(argument)) {
            stc("Syntax: medit show <vnum>\n\r", ch);
            return;
        }
        pMob = get_mob_index(atoi(argument));
        if(!pMob) {
            stc("Нет такого моба.\n\r", ch);
            return;
        }
        
        if (!OLCState::can_edit(ch, pMob->vnum)) {
            stc("У тебя недостаточно прав для редактирования монстров.\n\r", ch);
            return;
        }

        OLCStateMobile::Pointer(NEW, pMob)->findCommand(ch, "show")->entryPoint(ch, "noweb");
        return;
    } else if (arg_is_strict(arg1, "load")) {
        if(!*argument || !is_number(argument)) {
            stc("Syntax: medit load <vnum>\n\r", ch);
            return;
        }
        pMob = get_mob_index(atoi(argument));
        if(!pMob) {
            stc("Нет такого моба.\n\r", ch);
            return;
        }
        
        if (!OLCState::can_edit(ch, pMob->vnum)) {
            stc("У тебя недостаточно прав для создания этого монстра.\n\r", ch);
            return;
        }
        
        mob = create_mobile( pMob );
        if (mob->in_room == 0)
            char_to_room( mob, ch->in_room );
        
        oldact("$c1 создает $C4!", ch, 0, mob, TO_ROOM);
        oldact("Ты создаешь $C4!", ch, 0, mob, TO_CHAR);
        return;

    } else if (arg_is_strict(arg1, "convert")) {
        // One-off conversion of affect bits. To be removed once migration is finished.
        int cnt = 0;

        for (int i = 0; i < MAX_KEY_HASH; i++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next) {
            bool mobChanged = false;
            Flags aff(pMob->affected_by, &affect_flags);

            if (!pMob->affects.empty())
                mobChanged = true;
            
            pMob->affects.clear();

            if (aff.isSet(AFF_BLIND))
                pMob->affects.set(gsn_blindness);
            if (aff.isSet(AFF_INVISIBLE))
                pMob->affects.set(gsn_invisibility);
            if (aff.isSet(AFF_IMP_INVIS))
                pMob->affects.set(gsn_improved_invis);
            if (aff.isSet(AFF_FADE))
                pMob->affects.set(gsn_fade);
            if (aff.isSet(AFF_CORRUPTION))
                pMob->affects.set(gsn_corruption);
            if (aff.isSet(AFF_POISON))
                pMob->affects.set(gsn_poison);
            if (aff.isSet(AFF_HIDE))
                pMob->affects.set(gsn_hide);
            if (aff.isSet(AFF_SLEEP))
                pMob->affects.set(gsn_sleep);
            if (aff.isSet(AFF_CHARM))
                pMob->affects.set(gsn_charm_person);
            if (aff.isSet(AFF_CALM))
                pMob->affects.set(gsn_calm);
            if (aff.isSet(AFF_PLAGUE))
                pMob->affects.set(gsn_plague);
            if (aff.isSet(AFF_WEAKEN))
                pMob->affects.set(gsn_weaken);
            if (aff.isSet(AFF_BERSERK))
                pMob->affects.set(gsn_berserk);
            if (aff.isSet(AFF_CAMOUFLAGE))
                pMob->affects.set(gsn_camouflage);
            if (aff.isSet(AFF_SANCTUARY)) {
                if (pMob->alignment <= -350)
                    pMob->affects.set(gsn_dark_shroud);
                else if (IS_SET(pMob->act, ACT_MAGE))
                    pMob->affects.set(gsn_stardust);
                else
                    pMob->affects.set(gsn_sanctuary);                
            }
            if (aff.isSet(AFF_HASTE))
                pMob->affects.set(gsn_haste);
            if (aff.isSet(AFF_PROTECT_EVIL) && pMob->alignment > -350)
                pMob->affects.set(gsn_protection_evil);
            if (aff.isSet(AFF_CORRUPTION))
                pMob->affects.set(gsn_corruption);
            if (aff.isSet(AFF_FAERIE_FIRE))
                pMob->affects.set(gsn_faerie_fire);
            if (aff.isSet(AFF_INFRARED))
                pMob->affects.set(gsn_infravision);
            if (aff.isSet(AFF_FLYING))
                pMob->affects.set(gsn_fly);            
            if (aff.isSet(AFF_CURSE))
                pMob->affects.set(gsn_curse);
            if (aff.isSet(AFF_REGENERATION))
                pMob->affects.set(gsn_regeneration);
            if (aff.isSet(AFF_SNEAK))
                pMob->affects.set(gsn_sneak);
            if (aff.isSet(AFF_PASS_DOOR))
                pMob->affects.set(gsn_pass_door);
            if (aff.isSet(AFF_PROTECT_GOOD) && pMob->alignment < 350)
                pMob->affects.set(gsn_protection_good);            
            if (aff.isSet(AFF_SLOW))
                pMob->affects.set(gsn_slow);
            if (aff.isSet(AFF_SWIM))
                pMob->affects.set(gsn_swim);

            if (!pMob->affects.empty())
                mobChanged = true;

            if (mobChanged) {
                pMob->area->changed = true;
                cnt++;
            }
        }

        ch->pecho("Converted aff bits for %d mobs.", cnt);
        return;
    }
    
    stc("OLCStateMobile:  There is no default mobile to edit.\n\r", ch);
}

void OLCStateMobile::changed( PCharacter *ch )
{
    if(mob.area)
        mob.area->changed = true;
}


/* $Id$
 *
 * ruffina, 2004
 */

#include "medit.h"

#include "char.h"
#include "grammar_entities_impl.h"
#include <pcharacter.h>
#include <npcharacter.h>
#include "race.h"
#include <object.h>
#include <affect.h>
#include "room.h"
#include "skillgroup.h"

#include "websocketrpc.h"
#include "comm.h"
#include "merc.h"
#include "interp.h"
#include "../anatolia/handler.h"
#include "act.h"
#include "mercdb.h"

#include "olc.h"
#include "security.h"
#include "feniatriggers.h"

#include "def.h"

OLC_STATE(OLCStateMobile);

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
    mob.new_format       = original->new_format;
    mob.area             = original->area;

    copyParameters( original );
    copyDescriptions( original );
}

void OLCStateMobile::copyDescriptions( MOB_INDEX_DATA *original )
{
    mob.player_name      = str_dup(original->player_name);
    mob.short_descr      = str_dup(original->short_descr);
    mob.long_descr       = str_dup(original->long_descr);
    mob.description      = str_dup(original->description);
    mob.material         = str_dup(original->material);
    mob.smell            = original->smell;

    mob.properties.clear( );
    for (Properties::const_iterator p = original->properties.begin( ); p != original->properties.end( ); p++)
        mob.properties.insert( *p );
}

void OLCStateMobile::copyParameters( MOB_INDEX_DATA *original )
{
    mob.group            = original->group;
    mob.act              = original->act;
    mob.affected_by      = original->affected_by;
    mob.add_affected_by  = original->add_affected_by;
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
    mob.race             = str_dup(original->race);
    mob.wealth           = original->wealth;
    mob.form             = original->form;
    mob.parts            = original->parts;
    mob.size             = original->size;
    mob.practicer.clear( );
    mob.practicer.set( original->practicer );
    mob.religion.clear();
    mob.religion.set(original->religion);
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
        
        original = new_mob_index();

        original->vnum = mob.vnum;
        original->area = mob.area;

        if (mob.vnum > top_vnum_mob)
            top_vnum_mob = mob.vnum;

        mob.act |= ACT_IS_NPC;
        iHash = (int) mob.vnum % MAX_KEY_HASH;
        original->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = original;
        top_mob_index++;
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

            if(!strcmp(victim->getNameP(), original->player_name))
                victim->setName(DLString(mob.player_name));

            if(victim->act == original->act)
                victim->act = mob.act;
            
            if(victim->affected_by == original->affected_by)
                victim->affected_by = mob.affected_by;
            
            if(victim->add_affected_by == original->add_affected_by)
                victim->add_affected_by = mob.add_affected_by;
            
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

            if(victim->size == original->size)
                victim->size             = mob.size;
        }
    }

    free_string(original->player_name);
    original->player_name      = mob.player_name;
    mob.player_name = 0;
    free_string(original->short_descr);
    original->short_descr      = mob.short_descr;
    mob.short_descr = 0;
    free_string(original->long_descr);
    original->long_descr       = mob.long_descr;
    mob.long_descr = 0;
    free_string(original->description);
    original->description      = mob.description;
    mob.description = 0;
    original->spec_fun.name    = mob.spec_fun.name;
    original->spec_fun.func    = spec_lookup( mob.spec_fun.name.c_str() );
    original->vnum             = mob.vnum;
    original->group            = mob.group;
    original->new_format       = mob.new_format;
    original->smell            = mob.smell;
    mob.smell.clear( );
    original->act              = mob.act;
    original->affected_by      = mob.affected_by;
    original->add_affected_by  = mob.add_affected_by;
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
    free_string( original->race );
    original->race             = mob.race;
    mob.race = 0;
    original->wealth           = mob.wealth;
    original->form             = mob.form;
    original->parts            = mob.parts;
    original->size             = mob.size;
    free_string(original->material);
    original->material         = mob.material;
    mob.material = 0;
    original->behavior         = mob.behavior;
    mob.behavior = 0;    
    original->practicer.clear( );
    original->practicer.set( mob.practicer );
    original->religion.clear();
    original->religion.set(mob.religion);

    for(wch = char_list; wch; wch = wch->next) {
        NPCharacter *victim = wch->getNPC();

        if (victim && victim->pIndexData == original) 
            victim->updateCachedNoun( );
    }


    original->properties.clear( );
    for (Properties::const_iterator p = mob.properties.begin( ); p != mob.properties.end( ); p++)
        original->properties.insert( *p );
    mob.properties.clear( );
}

void OLCStateMobile::statePrompt(Descriptor *d)
{
    d->send( "Editing mob> " );
}

// Mobile Editor Functions.
MEDIT(show)
{
    bool showWeb = !arg_oneof_strict(argument, "noweb");

    ptc(ch, "{GName: [{x%s{G] %s\n\r", 
        mob.player_name, web_edit_button(showWeb, ch, "name", "web").c_str());

    ptc(ch, "{GShort desc: {x%s %s\n\r{GLong descr: %s{x\n\r%s",
        mob.short_descr, web_edit_button(showWeb, ch, "short", "web").c_str(),
        web_edit_button(showWeb, ch, "long", "web").c_str(), mob.long_descr);        
    
    ptc(ch, "{CLevel {Y%3d  {CVnum: [{Y%u{C]  Area: [{Y%5d{C] {G%s{x\n\r",
        mob.level, mob.vnum,
        !mob.area ? -1 : mob.area->vnum,
        !mob.area ? "No Area" : mob.area->name);

    ptc(ch, "Race [{G%s{x] {D(? race){x Sex: [{G%s{x] {D(? sex_table){x Number: [{G%s{x]\n\r",
        mob.race, 
        sex_table.name( mob.sex ).c_str( ),
        mob.gram_number == Grammar::Number::PLURAL ? "plural" : "singular");

    ptc(ch, "{GHit dice:{x    [%6dd%-5d+%6d]\n\r",
        mob.hit[DICE_NUMBER],
        mob.hit[DICE_TYPE],
        mob.hit[DICE_BONUS]);

    ptc(ch, "{RDamage dice:{x [%6dd%-5d+%6d]\n\r",
        mob.damage[DICE_NUMBER],
        mob.damage[DICE_TYPE],
        mob.damage[DICE_BONUS]);

    ptc(ch, "{CMana dice:{x   [%6dd%-5d+%6d]\n\r",
        mob.mana[DICE_NUMBER],
        mob.mana[DICE_TYPE],
        mob.mana[DICE_BONUS]);

    ptc(ch, "Wealth:[%10d]   Align:  [%5d]\n\r",
        mob.wealth, mob.alignment);

    ptc(ch, "Hitroll:[%5d] Damage:[%10s] {D(? weapon_flags){x\n\r",
        mob.hitroll, weapon_flags.name(mob.dam_type).c_str( ));

    ptc(ch, "Armor: [pierce: %d  bash: %d  slash: %d  magic: %d] {D(? ac_type){x\n\r",
        mob.ac[AC_PIERCE], mob.ac[AC_BASH],
        mob.ac[AC_SLASH], mob.ac[AC_EXOTIC]);

    ptc(ch, "Act: [{R%s{x] {D(? act_flags){x\n\r", act_flags.names(mob.act).c_str());

    ptc(ch, "Aff: [{C%s{x] {D(? affect_flags){x\n\r", affect_flags.names(mob.affected_by).c_str());
    ptc(ch, "Det: [{M%s{x] {D(? detect_flags){x\n\r", detect_flags.names(mob.detection).c_str());

    ptc(ch, "Pos   : starting [{Y%s{x]  default [{Y%s{x] {D(? position_table){x\n\r",
        position_table.name(mob.start_pos).c_str(),
        position_table.name(mob.default_pos).c_str());

    ptc(ch, "Imm:  [{W%s{x] {D(? imm_flags){x\n\r", imm_flags.names(mob.imm_flags).c_str());
    ptc(ch, "Res:  [{Y%s{x] {D(? res_flags){x\n\r", res_flags.names(mob.res_flags).c_str());
    ptc(ch, "Vuln: [{y%s{x] {D(? vuln_flags){x\n\r", vuln_flags.names(mob.vuln_flags).c_str());
    ptc(ch, "Off:  [{M%s{x] {D(? off_flags){x\n\r", off_flags.names(mob.off_flags).c_str());
    ptc(ch, "Size: [{G%s{x] {D(? size_table){x\n\r", size_table.name(mob.size).c_str());

    ptc(ch, "Material: [%s] {D(? material){x\n\r", mob.material);
    ptc(ch, "Form:     [%s] {D(? form_flags){x\n\r", form_flags.names(mob.form).c_str());
    ptc(ch, "Parts:    [%s] {D(? part_flags){x\n\r", part_flags.names(mob.parts).c_str());

    if (!mob.spec_fun.name.empty())
        ptc(ch, "Spec fun: [%s] {D(? spec){x\n\r", mob.spec_fun.name.c_str());
    ptc(ch, "Group:    [%d]\n\r", mob.group);
    ptc(ch, "Practicer:[{G%s{x] {D(? groups){x\n\r", mob.practicer.toString( ).c_str( ));
    ptc(ch, "Religion: [{G%s{x] {D(reledit list){x\n\r", mob.religion.toString().c_str());
    ptc(ch, "Smell:     %s\n\r", mob.smell.c_str( ));

    if (!mob.properties.empty( )) {
        ptc(ch, "Properties:\n\r");
        for (Properties::const_iterator p = mob.properties.begin( ); p != mob.properties.end( ); p++)
            ptc(ch, "%20s: %s\n\r", p->first.c_str( ), p->second.c_str( ));
    }

    ptc(ch, "Description: %s\n\r%s", web_edit_button(showWeb, ch, "desc", "web").c_str(), mob.description);

    if (mob.behavior) {
        try {
            std::basic_ostringstream<char> ostr;
            mob.behavior->save( ostr );
            ptc(ch, "Behavior:\r\n%s\r\n", ostr.str( ).c_str( ));
            
        } catch (const ExceptionXMLError &e) {
            ptc(ch, "Behavior is BUGGY.\r\n");
        }
    }

    MOB_INDEX_DATA *original = get_mob_index(mob.vnum);
    if (original)
        show_fenia_triggers(ch, original->wrapper);
    feniaTriggers->showAvailableTriggers(ch, "mob");
    return false;
}

MEDIT(fenia)
{
    feniaTriggers->openEditor(ch, mob, argument);
    return false;
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
        stc("Поведение продавца не задано, используйте 'behavior shopper'.\r\n", ch);
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
    ptc(ch, "%s находится в:\r\n", DLString( mob.short_descr ).ruscase('1').c_str( ));

    for (Character *wch = char_list; wch; wch = wch->next) {
        if (!wch->is_npc( ))
            continue;

        if (wch->getNPC( )->pIndexData->vnum != mob.vnum)
            continue;

        ptc(ch, "[%5d]    %-30s (%s)\r\n", 
                wch->in_room->vnum,
                wch->in_room->name, 
                wch->in_room->area->name);
    }

    return true;
}

MEDIT(spec)
{
    if (argument[0] == '\0') {
        stc("Syntax:  spec [special function]\n\r", ch);
        return false;
    }


    if (!str_cmp(argument, "none")) {
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
    return editor(argument, mob.description);
}

MEDIT(smell)
{
    if (!*argument) {
        if(sedit(mob.smell)) {
            stc("Smell set\n\r", ch);
            return true;
        } else
            return false;
    }

    mob.smell = argument;
    stc("Smell set\n\r", ch);
    return false;
}

MEDIT(property)
{
    DLString args = DLString( argument );
    return mapEdit( mob.properties, args );
}

MEDIT(behavior)
{
    DLString type;

    if (!*argument) {
        if(!xmledit(mob.behavior))
            return false;

        stc("Поведение установлено.\r\n", ch);
        return true;
    }

    if (mob.behavior) {
        if (!str_cmp( argument, "clear" )) {
            mob.behavior.clear( );
            stc("Поведение очищено.\r\n", ch);
            return true;
        }

        stc("Поведение уже задано, используйте 'behavior' для редактирования или 'behavior clear' для очистки.\r\n", ch);
        return false;
    }

    if (!str_cmp( argument, "shopper" )) {
        static const DLString shopperType( "ShopTrader" );
        type = shopperType;
    }
    else if (!str_cmp( argument, "pet" )) {
        static const DLString petType( "Pet" );
        type = petType;
    }
    else if (!str_cmp( argument, "leveladaptivepet" )) {
        static const DLString petType( "LevelAdaptivePet" );
        type = petType;
    }
    else if (!str_cmp( argument, "trainer" )) {
        static const DLString trainerType( "Trainer" );
        type = trainerType;
    }
    else { 
        stc("Допустимые значения поведения: shopper, pet, leveladaptivepet, trainer.\r\n", ch);
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

MEDIT(long)
{
    return editor(argument, mob.long_descr, (editor_flags)(ED_UPPER_FIRST_CHAR|ED_ADD_NEWLINE));
}

MEDIT(short)
{
    return editor(argument, mob.short_descr, ED_NO_NEWLINE);
}

MEDIT(name)
{
    return editor(argument, mob.player_name, ED_NO_NEWLINE);
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

MEDIT(affect)
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
    if (argument[0] == '\0') {
        stc("Syntax:  material [string]\n\r", ch);
        return false;
    }

    free_string(mob.material);
    mob.material = str_dup(argument);

    stc("Material set.\n\r", ch);
    return true;
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

            free_string( mob.race );
            mob.race = str_dup( race->getName( ).c_str( ) );

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
    
    ostringstream out;
    raceManager->outputAll( out, 16, 3 );
    stc(out.str( ).c_str( ), ch );
    stc("\n\r", ch);
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
    int i, cnt;
    RoomIndexData *pRoom;
    RESET_DATA *pReset;
    char buf[MAX_STRING_LENGTH];
    ostringstream buffer;
    
    snprintf(buf, sizeof(buf), "Resets for mobile [{W%d{x] ({g%s{x):\n\r",
            mob.vnum, 
            russian_case(mob.short_descr, '1').c_str( ));
    buffer << buf;
    
    cnt = 0;
    for(i=0;i<MAX_KEY_HASH;i++)
        for(pRoom = room_index_hash[i];pRoom;pRoom = pRoom->next) 
            for(pReset = pRoom->reset_first;pReset;pReset = pReset->next)
                switch(pReset->command) {
                    case 'M':
                        if(pReset->arg1 == mob.vnum) {
                            snprintf(buf, sizeof(buf), "{G%c{x in room [{W%d{x] ({g%s{x)\n\r",
                                    pReset->command, pRoom->vnum, pRoom->name);
                            buffer << buf;
                            cnt++;
                        }
                }

    snprintf(buf, sizeof(buf), "Total {W%d{x resets found.\n\r", cnt);
    buffer << buf;
    
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
                    "  copy param <vnum> -- copy race, hit, damage and other parameters from <vnum> mob index.\r\n"
                    "  copy desc <vnum>  -- copy name, description, short and long description from <vnum> mob index.\r\n" );
        return false;
    }
    
    original = get_mob_index( arg2.toInt() );

    if (original == NULL) {
        ch->println("Mobile not found.\r\n");
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

MEDIT(average)
{
    DLString arg = DLString(argument).getOneArgument();
    bool fAll, fDone;
    int delta, minLevel, maxLevel;
    int hitBonus = 0, hitType = 0, hitNumber = 0;
    int manaBonus = 0, manaType = 0, manaNumber = 0;
    int hitroll = 0, damBonus = 0, damType = 0, damNumber = 0;
    int ac0 = 0, ac1 = 0, ac2 = 0, ac3 = 0;
    int total;
    
    if (mob.level == 0) {
        ch->println("Please set non-zero mob level.");
        return false;
    }
    
    delta = mob.level / 20;
    minLevel = max( 1, mob.level - delta );
    maxLevel = mob.level + delta;
    total = 0;

    for (int iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[iHash]; pMob; pMob = pMob->next) {
            if (IS_SET(pMob->area->area_flag, AREA_NOQUEST|AREA_HIDDEN|AREA_SYSTEM))
                continue;
            if (pMob->vnum == 3174) // old jew
                continue;
            if (pMob->level > maxLevel || pMob->level < minLevel)
                continue;

            total++;

            hitBonus += pMob->hit[DICE_BONUS];
            hitType += pMob->hit[DICE_TYPE];
            hitNumber += pMob->hit[DICE_NUMBER];
            manaBonus += pMob->mana[DICE_BONUS];
            manaType += pMob->mana[DICE_TYPE];
            manaNumber += pMob->mana[DICE_NUMBER];
            hitroll += pMob->hitroll;
            damBonus += pMob->damage[DICE_BONUS];
            damType += pMob->damage[DICE_TYPE];
            damNumber += pMob->damage[DICE_NUMBER];
            ac0 += pMob->ac[0];
            ac1 += pMob->ac[1];
            ac2 += pMob->ac[2];
            ac3 += pMob->ac[3];
        }
    
    if (total == 0) {
        ch->printf("No mobiles found in level range %d-%d.\r\n", minLevel, maxLevel);
        return false;
    }
     
    fAll = (arg.empty() || arg == "all");
    fDone = false;

    if (fAll || arg == "hit" || arg == "hp") {
        mob.hit[DICE_BONUS] = hitBonus / total;
        mob.hit[DICE_TYPE] = hitType / total;
        mob.hit[DICE_NUMBER] = hitNumber / total;
        fDone = true;
    }
    if (fAll || arg.strPrefix("mana")) {
        mob.mana[DICE_BONUS] = manaBonus / total;
        mob.mana[DICE_TYPE] = manaType / total;
        mob.mana[DICE_NUMBER] = manaNumber / total;
        fDone = true;
    }
    if (fAll || (arg.strPrefix("hitroll") && arg.size() > 3)) {
        mob.hitroll = hitroll / total;
        fDone = true;
    }
    if (fAll || arg.strPrefix("damage")) {
        mob.damage[DICE_BONUS] = damBonus / total;
        mob.damage[DICE_TYPE] = damType / total;
        mob.damage[DICE_NUMBER] = damNumber / total;
        fDone = true;
    }
    if (fAll || arg.strPrefix("armor") || arg == "ac") {
        mob.ac[0] = ac0 / total;
        mob.ac[1] = ac1 / total;
        mob.ac[2] = ac2 / total;
        mob.ac[3] = ac3 / total;
        fDone = true;
    }
    
    if (fDone) {
        ch->println("Average values assigned.\r\n");
        return true;
    }
    else {
        ch->send_to("Syntax: \r\n"
                    "   average hp|mana|hitroll|damage|ac -- setup one param\r\n"
                    "   average [all] -- setup all parameters\r\n");
        return false;
    }
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
        me->findCommand(ch, "show")->run(ch, "");
        return;
    }
    else if (!str_cmp(arg1, "create")) {
        if (!str_cmp(argument, "next")) {
            value = next_mob_index(ch, ch->in_room->pIndexData);
            if (value < 0) {
                ch->println("Все внумы в этой зоне уже заняты!");
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
    } else if(!str_cmp(arg1, "show")) {
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

        OLCStateMobile::Pointer(NEW, pMob)->findCommand(ch, "show")->run(ch, "noweb");
        return;
    } else if (!str_cmp(arg1, "load")) {
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
        
        act_p( "$c1 создает $C4!", ch, 0, mob, TO_ROOM,POS_RESTING );
        act_p( "Ты создаешь $C4!", ch, 0, mob, TO_CHAR,POS_RESTING );
        return;
    }
    
    stc("OLCStateMobile:  There is no default mobile to edit.\n\r", ch);
}

void OLCStateMobile::changed( PCharacter *ch )
{
    if(mob.area)
        SET_BIT(mob.area->area_flag, AREA_CHANGED);
}


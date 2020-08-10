/* $Id$
 *
 * ruffina, 2004
 */

#include "olc.h"

#include "mercdb.h"

#include "character.h"
#include "merc.h"
#include "skill.h"
#include "skillmanager.h"
#include "loadsave.h"
#include "recipeflags.h"
#include "act_lock.h"
#include "weapons.h"
#include "liquid.h"
#include "def.h"

static int skill_lookup( const DLString &name )
{
    Skill *skill = skillManager->findExisting( name );

    if (skill)
       return skill->getIndex( );
    else
       return -1;
}

// Object Editor Functions.
void show_obj_values(Character * ch, OBJ_INDEX_DATA * obj)
{
    char buf[MAX_STRING_LENGTH];

    switch (obj->item_type) {
    default:
        break;
    
    case ITEM_SPELLBOOK:
        ptc(ch, "[v0] Total pages       : [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Used pages        : [%d]\n\r", obj->value[1]);
        ptc(ch, "[v2] Max spell quality : [%d]\n\r", obj->value[2]);
        ptc(ch, "[v3] Spell to learn    : %s {D(? spells){x\n\r", get_skill_name( obj->value[3] ));
        ptc(ch, "[v4] Spell to learn    : %s\n\r", get_skill_name( obj->value[4] ));
        break;

    case ITEM_TEXTBOOK:
        ptc(ch, "[v0] Total pages       : [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Used pages        : [%d]\n\r", obj->value[1]);
        ptc(ch, "[v2] Max skill quality : [%d]\n\r", obj->value[2]);
        ptc(ch, "[v3] Skill to learn    : %s {D(? spells){x\n\r", get_skill_name( obj->value[3] ));
        ptc(ch, "[v4] Skill to learn    : %s\n\r", get_skill_name( obj->value[4] ));
        break;

    case ITEM_RECIPE:
        ptc(ch, "[v0] Recipe flags      : %s {D(? recipe_flags){x\r\n", recipe_flags.names(obj->value[0]).c_str());
        ptc(ch, "[v2] Complexity        : [%d]\r\n", obj->value[2]);
        break;

    case ITEM_MAP:
        ptc(ch, "[v0] Не исчезает в руках    : [%d]\n\r", obj->value[0]);
        break;

    case ITEM_KEY:
        ptc(ch, "[v0] Не исчезает в руках    : [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Исчезает на земле      : [%d]\n\r", obj->value[1]);
        break;

    case ITEM_KEYRING:
        ptc(ch, "[v0] Max key count: [%d]\n\r", obj->value[0]);
        break;

    case ITEM_LOCKPICK:
        ptc(ch, "[v0] %s [%d]\n\r", 
            (obj->value[0] == Keyhole::LOCK_VALUE_MULTI ? 
                    "Opens all locks" : 
             obj->value[0] == Keyhole::LOCK_VALUE_BLANK ? 
                    "Used as a blank for key forgery" :
                    "Opens locks of type"),
            obj->value[0]);
        ptc(ch, "[v1] Quality [%d]\n\r", obj->value[1]);
        break;

    case ITEM_LIGHT:
        sprintf(buf, "[v2] Light:  [%d]\n\r", obj->value[2]);
        stc(buf, ch);
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        sprintf(buf,
                  "[v0] Level:          [%u]\n\r"
                  "[v1] Charges Total:  [%u]\n\r"
                  "[v2] Charges Left:   [%u]\n\r"
                  "[v3] Spell:          %s {D(? spells){x\n\r",
                  obj->value[0],
                  obj->value[1],
                  obj->value[2],
                  get_skill_name( obj->value[3] ));
        stc(buf, ch);
        break;

    case ITEM_PORTAL:
        sprintf(buf,
                  "[v0] Charges:        [%u]\n\r"
                  "[v1] Exit Flags:     %s {D(? exit_flags){x\n\r"
                  "[v2] Portal Flags:   %s {D(? portal_flags){x\n\r"
                  "[v3] Goes to (vnum): [%u]\n\r"
                  "[v4] Has key(vnum):  [%d]\n\r",
                  obj->value[0],
                  exit_flags.names(obj->value[1]).c_str(),
                  portal_flags.names(obj->value[2]).c_str(),
                  obj->value[3], obj->value[4]);
        stc(buf, ch);
        break;

    case ITEM_FURNITURE:
        sprintf(buf,
                  "[v0] Max people:      [%u]\n\r"
                  "[v1] Max weight:      [%u]\n\r"
                  "[v2] Furniture Flags: %s {D(? furniture_flags){x\n\r"
                  "[v3] Heal bonus:      [%u]\n\r"
                  "[v4] Mana bonus:      [%u]\n\r",
                  obj->value[0],
                  obj->value[1],
                  furniture_flags.names(obj->value[2]).c_str(),
                  obj->value[3],
                  obj->value[4]);
        stc(buf, ch);
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        sprintf(buf,
                  "[v0] Level:  [%u]\n\r"
                  "[v1] Spell:  %s {D(? spells){x\n\r"
                  "[v2] Spell:  %s\n\r"
                  "[v3] Spell:  %s\n\r"
                  "[v4] Spell:  %s\n\r",
                  obj->value[0],
                  get_skill_name( obj->value[1] ),
                  get_skill_name( obj->value[2] ),
                  get_skill_name( obj->value[3] ),
                  get_skill_name( obj->value[4] ) );
        stc(buf, ch);
        break;

    case ITEM_ARMOR:
        sprintf(buf,
                  "[v0] Ac pierce       [%u]\n\r"
                  "[v1] Ac bash         [%u]\n\r"
                  "[v2] Ac slash        [%u]\n\r"
                  "[v3] Ac exotic       [%u]\n\r",
                  obj->value[0],
                  obj->value[1],
                  obj->value[2],
                  obj->value[3]);
        stc(buf, ch);
        break;

    case ITEM_WEAPON:
        ptc(ch, "[v0] Вид оружия:   %s {D(? weapon_class){x\n\r",
            weapon_class.name(obj->value[0]).c_str());
        ptc(ch, "[v1] Число бросков кубика: [%u]\n\r", obj->value[1]);
        ptc(ch, "[v2] Число граней кубика : [%u] {D(повреждения %ud%u, среднее %u = (v2+1)*v1/2){x\r\n", 
                       obj->value[2], obj->value[1], obj->value[2], weapon_ave(obj));
        ptc(ch, "[v3] Тип удара:    %s {D(? weapon_flags){x\n\r",
            weapon_flags.name(obj->value[3]).c_str());
        ptc(ch, "[v4] Флаги оружия: %s {D(? weapon_type2){x\n\r",
            weapon_type2.names(obj->value[4]).c_str());
        break;



    case ITEM_CONTAINER:
        ptc(ch,
            "[v0] Вместимость:[%u кг]\n\r"
            "[v1] Флаги:      [%s] {D(? container_flags){x\n\r"
            "[v2] Ключ:        %s [%d]\n\r"
            "[v3] Макс. вес:  [%u]\n\r"
            "[v4] Коэф. снижения веса: [%u]\n\r",
            obj->value[0],
            container_flags.names(obj->value[1]).c_str(),
            (obj->value[2] > 0 && get_obj_index(obj->value[2])) ? get_obj_index(obj->value[2])->short_descr
            : "none",
            obj->value[2],
            obj->value[3],
            obj->value[4]);
        break;

    case ITEM_DRINK_CON:
        ptc(ch,
            "[v0] Всего жидкости:  [%u]\n\r"
            "[v1] Налито жидкости: [%u]\n\r"
            "[v2] Тип жидкости:     %s (%s) {D(? liquid){x\n\r"
            "[v3] Флаги:            %s {D(? drink_flags){x\n\r"
            "[v4] Пробка:           %u\n\r",
            obj->value[0],
            obj->value[1],
            liquidManager->find( obj->value[2] )->getName( ).c_str( ),
            liquidManager->find( obj->value[2] )->getShortDescr( ).ruscase( '1' ).c_str( ),
            drink_flags.names(obj->value[3]).c_str( ),
            obj->value[4]);
        break;

    case ITEM_FOUNTAIN:
        ptc(ch, 
            "[v0] Liquid Total: [%u]\n\r"
            "[v1] Liquid Left:  [%u]\n\r"
            "[v2] Liquid:       %s (%s) {D(? liquid){x\n\r",
            obj->value[0],
            obj->value[1],
            liquidManager->find( obj->value[2] )->getName( ).c_str( ),
            liquidManager->find( obj->value[2] )->getShortDescr( ).ruscase( '1' ).c_str( ));
        break;

    case ITEM_FOOD:
        ptc(ch,
            "[v0] Food hours: [%u]\n\r"
            "[v1] Full hours: [%u]\n\r"
            "[v3] Poisoned:   %s\n\r",
            obj->value[0],
            obj->value[1],
            obj->value[3] != 0 ? "Yes" : "No");
        break;

    case ITEM_MONEY:
        ptc(ch, "[v0] Silver:   [%u]\n\r", obj->value[0]);
        ptc(ch, "[v1] Gold:     [%u]\n\r", obj->value[1]);
        break;

    }
}

bool set_obj_values(Character * ch, OBJ_INDEX_DATA * pObj, int value_num, const char *argument)
{
    Liquid *liq;

    switch (pObj->item_type) {
    default:
        stc("Value not used.\n\r", ch);
        return false;
   
   case ITEM_SPELLBOOK:
   case ITEM_TEXTBOOK:
        switch (value_num) {
        default:
            stc("Value not used\r\n\r", ch);
            return false;
        case 0:
            stc("Total number of pages set\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Number of used pages set\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("Max spell/skill quality set\r\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
        case 4:
            stc("Skill/spell to learn set.\r\n\r", ch);
            pObj->value[value_num] = skill_lookup(argument);
            break;
        }
        break;
       
    case ITEM_RECIPE:
        switch (value_num) {
            int value;
        default:
            stc("Value not used\r\n\r", ch);
            return false;
        case 0:
            if ((value = recipe_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[0], value);
            else {
                show_help(ch, "recipe");
                return false;
            }
            stc("Recipe flags toggled.\n\r\n\r", ch);
            break;
        case 2:
            stc("Recipe complexity set.\r\n", ch);
            pObj->value[2] = atoi(argument);
            break;
        }
        break;
 
    case ITEM_MAP:
        switch (value_num) {
            int i;

        default:
            stc("Value not used.\n\r", ch);
            return false;
        case 0: 
            i = atoi( argument );
            if (i)
                stc("Will not rot in inventory.\n\r", ch);
            else 
                stc("Will rot in inventory.\n\r", ch);

            pObj->value[0] = i;
            break;
        }
        break;
    case ITEM_KEYRING: 
        switch (value_num) {
        default:
            stc("Value not used.\n\r", ch);
            return false;
        case 0: 
            pObj->value[0] = atoi( argument );
            ptc(ch, "The keyring will hold maximum %d keys.\n\r", pObj->value[0]);
        }
        break;
    case ITEM_LOCKPICK: 
        switch (value_num) {
        int i;

        default:
            stc("Value not used.\n\r", ch);
            return false;
        case 0: 
            i = atoi( argument );
            
            if (i == Keyhole::LOCK_VALUE_MULTI)
                stc("The lockpick will open any type of lock.\n\r", ch);
            else if (i == Keyhole::LOCK_VALUE_BLANK)
                stc("This will be a blank for a lockpick.\n\r", ch);
            else if (i < 0 || i >= Keyhole::MAX_KEY_TYPES) {
                stc("Invalid lock type.\n\r", ch);
                return false;
            }
            else
                ptc(ch, "The lockpick will open locks of type %d.\n\r", i);

            pObj->value[0] = i;
            break;
        case 1: 
            i = atoi( argument );
            if (i < 0 || i > 100) {
                stc("Invalid quality, valid range 0..100.\n\r", ch);
                return false;
            }
            
            ptc(ch, "The lockpick quality set to %d%%.\n\r", i);
            pObj->value[1] = i;
            break;
        }
        break;
    case ITEM_KEY: 
        switch (value_num) {
        int i;

        default:
            stc("Value not used.\n\r", ch);
            return false;
        case 0: 
            i = atoi( argument );
            if (i) {
                if (i == 1)
                    stc("Key will not rot in inventory.\n\r", ch);
                else
                    ptc(ch, "Key will be kept in inventory for %d minutes.\r\n", i);
            }
            else 
                stc("Key will rot in inventory.\n\r", ch);

            pObj->value[0] = i;
            break;
        case 1: 
            i = atoi( argument );
            if (i)
                stc("Key will rot on the ground.\n\r", ch);
            else 
                stc("Key will not rot on the ground.\n\r", ch);

            pObj->value[1] = i;
            break;
        }
        break;
        
    case ITEM_LIGHT:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_LIGHT");
            return false;
        case 2:
            stc("HOURS OF LIGHT SET.\n\r\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        }
        break;
    case ITEM_WAND:
    case ITEM_STAFF:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_STAFF_WAND");
            return false;
        case 0:
            stc("SPELL LEVEL SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("SPELL TYPE SET.\n\r", ch);
            pObj->value[3] = skill_lookup(argument);
            break;
        }
        break;
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_SCROLL_POTION_PILL");
            return false;
        case 0:
            stc("SPELL LEVEL SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("SPELL TYPE 1 SET.\n\r\n\r", ch);
            pObj->value[1] = skill_lookup(argument);
            break;
        case 2:
            stc("SPELL TYPE 2 SET.\n\r\n\r", ch);
            pObj->value[2] = skill_lookup(argument);
            break;
        case 3:
            stc("SPELL TYPE 3 SET.\n\r\n\r", ch);
            pObj->value[3] = skill_lookup(argument);
            break;
        case 4:
            stc("SPELL TYPE 4 SET.\n\r\n\r", ch);
            pObj->value[4] =  skill_lookup(argument);
            break;
        }
        break;
    case ITEM_ARMOR:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_ARMOR");
            return false;
        case 0:
            stc("AC PIERCE SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("AC BASH SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("AC SLASH SET.\n\r\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("AC EXOTIC SET.\n\r\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        }
        break;
    case ITEM_WEAPON:
        switch (value_num) {
            int value;
        default:
//            do_help(ch, "ITEM_WEAPON");
            return false;
        case 0:
            stc("WEAPON CLASS SET.\n\r\n\r", ch);
            pObj->value[0] = weapon_class.value( argument );
            break;
        case 1:
            stc("NUMBER OF DICE SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("TYPE OF DICE SET.\n\r\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("WEAPON TYPE SET.\n\r\n\r", ch);
            pObj->value[3] = weapon_flags.value( argument );
            break;
        case 4:
            if ((value = weapon_type2.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[value_num], value);
            else {
                show_help(ch, "weapon_type2");
                return false;
            }
            stc("Special weapon type toggled.\n\r\n\r", ch);
            break;
        }
        break;
    case ITEM_PORTAL:
        switch (value_num) {
            int value;
        default:
//            do_help(ch, "ITEM_PORTAL");
            return false;
        case 0:
            stc("CHARGES SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            if ((value = exit_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[value_num], value);
            else {
                show_help(ch, "exit");
                return false;
            }
            stc("Exit flags toggled.\n\r\n\r", ch);
            break;
        case 2:
            if ((value = portal_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[2], value);
            else {
                show_help(ch, "portal");
                return false;
            }
            stc("Portal flags toggled.\n\r\n\r", ch);
            break;
        case 3:
            stc("EXIT VNUM SET.\n\r\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            stc("Key vnum set.\n\r\n\r", ch);
            pObj->value[4] = atoi(argument);
        }
        break;
    case ITEM_FURNITURE:
        switch (value_num) {
            int value;
        default:
//            do_help(ch, "ITEM_FURNITURE");
            return false;
        case 0:
            stc("NUMBER OF PEOPLE SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("MAX WEIGHT SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            if ((value = furniture_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[2], value);
            else {
                show_help(ch, "furniture");
                return false;
            }
            stc("Furniture flags toggled.\n\r\n\r", ch);
            break;
        case 3:
            stc("HEAL BONUS SET.\n\r\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            stc("MANA BONUS SET.\n\r\n\r", ch);
            pObj->value[4] = atoi(argument);
            break;
        }
        break;
    case ITEM_CONTAINER:
        switch (value_num) {
            int value;

        default:
//            do_help(ch, "ITEM_CONTAINER");
            return false;
        case 0:
            stc("CAPACITY SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            if ((value = container_flags.bitstring( argument ))
                != NO_FLAG)
                TOGGLE_BIT(pObj->value[1], value);
            else {
                show_help(ch, "container");
                return false;
            }
            stc("Container flags toggled.\n\r\n\r", ch);
            break;
        case 2:
            if (atoi(argument) > 0) {
                if (!get_obj_index(atoi(argument))) {
                    stc("THERE IS NO SUCH ITEM.\n\r\n\r", ch);
                    return false;
                }
                if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
                    stc("THAT ITEM IS NOT A KEY.\n\r\n\r", ch);
                    return false;
                }
            }
            stc("CONTAINER KEY SET.\n\r\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("CONTAINER MAX WEIGHT SET.\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            stc("WEIGHT MULTIPLIER SET.\n\r\n\r", ch);
            pObj->value[4] = atoi(argument);
            break;
        }
        break;
    case ITEM_DRINK_CON:
        switch (value_num) {
            int value;

        default:
//            do_help(ch, "ITEM_DRINK");
            return false;
        case 0:
            stc("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            if (!( liq = liquidManager->findExisting( argument ) )) {
                stc("Invalid liquid name, use '? liquid'.\r\n", ch);
                return false;
            }
            pObj->value[2] = liq->getIndex( );
            stc("LIQUID TYPE SET.\n\r\n\r", ch);
            break;
        case 3:
            if ((value = drink_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[3], value);
            else {
                show_help(ch, "drink");
                return false;
            }
            stc("Drink-container flags toggled.\n\r\n\r", ch);
            break;
        case 4:
            stc("Corkscrew vnum set.\n\r\n\r", ch);
            pObj->value[4] = atoi(argument);
            break;
        }
        break;
    case ITEM_FOUNTAIN:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_FOUNTAIN");
            return false;
        case 0:
            stc("MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            if (!( liq = liquidManager->findExisting( argument ) )) {
                stc("Invalid liquid name, use '? liquid'.\r\n", ch);
                return false;
            }
            pObj->value[2] = liq->getIndex( );
            stc("LIQUID TYPE SET.\n\r\n\r", ch);
            break;
        }
        break;
    case ITEM_FOOD:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_FOOD");
            return false;
        case 0:
            stc("HOURS OF FOOD SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("HOURS OF FULL SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 3:
            stc("POISON VALUE TOGGLED.\n\r\n\r", ch);
            pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
            break;
        }
        break;
    case ITEM_MONEY:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_MONEY");
            return false;
        case 0:
            stc("SILVER AMOUNT SET.\n\r\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("GOLD AMOUNT SET.\n\r\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        }
        break;
    }
    show_obj_values(ch, pObj);
    return true;
}


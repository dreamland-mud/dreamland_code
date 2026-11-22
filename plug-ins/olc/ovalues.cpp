/* $Id$
 *
 * ruffina, 2004
 */

#include "olc.h"
#include "olcstate.h"
#include "ovalues.h"



#include "character.h"
#include "merc.h"
#include "skill.h"
#include "skillmanager.h"
#include "loadsave.h"
#include "recipeflags.h"
#include "keyhole.h"
#include "weapons.h"
#include "liquid.h"
#include "weapontier.h"
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
void show_obj_values(Character * ch, OBJ_INDEX_DATA * obj, bool showWeb)
{
    bool random;
    OBJ_INDEX_DATA *pKey;

    switch (obj->item_type) {
    default:
        break;
    
    case ITEM_SPELLBOOK:
        ptc(ch, "[v0] Всего страниц     : [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Занято страниц    : [%d]\n\r", obj->value[1]);
        ptc(ch, "[v2] Макс. качество    : [%d]\n\r", obj->value[2]);
        ptc(ch, "[v3] Заклинание        : %s {D(? spells){x\n\r", get_skill_name( obj->value[3] ));
        ptc(ch, "[v4] Заклинание        : %s\n\r", get_skill_name( obj->value[4] ));
        break;

    case ITEM_TEXTBOOK:
        ptc(ch, "[v0] Всего страниц     : [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Занято страниц    : [%d]\n\r", obj->value[1]);
        ptc(ch, "[v2] Макс. качество    : [%d]\n\r", obj->value[2]);
        ptc(ch, "[v3] Умение            : %s {D(? spells){x\n\r", get_skill_name( obj->value[3] ));
        ptc(ch, "[v4] Умение            : %s\n\r", get_skill_name( obj->value[4] ));
        break;

    case ITEM_RECIPE:
        ptc(ch, "[v0] Флаги рецепта     : %s {D(? recipe_flags){x\r\n", web_flag_menu(showWeb, recipe_flags, "v0", recipe_flags.names(obj->value[0]), OLC_FRAME_NONE).c_str());
        ptc(ch, "[v2] Сложность         : [%d]\r\n", obj->value[2]);
        break;

    case ITEM_MAP:
        ptc(ch, "[v0] Не исчезает в руках    : [%d]\n\r", obj->value[0]);
        break;

    case ITEM_KEY:
        ptc(ch, "[v0] Не исчезает в руках    : [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Исчезает на земле      : [%d]\n\r", obj->value[1]);
        break;

    case ITEM_KEYRING:
        ptc(ch, "[v0] Макс. ключей: [%d]\n\r", obj->value[0]);
        break;

    case ITEM_LOCKPICK:
        ptc(ch, "[v0] %s [%d]\n\r", 
            (obj->value[0] == Keyhole::LOCK_VALUE_MULTI ? 
                    "Открывает все замки" : 
             obj->value[0] == Keyhole::LOCK_VALUE_BLANK ? 
                    "Заготовка для подделки ключа" :
                    "Открывает замки типа"),
            obj->value[0]);
        ptc(ch, "[v1] Качество [%d]\n\r", obj->value[1]);
        break;

    case ITEM_LIGHT:
        ptc(ch, "[v2] Свет, часов: [%d]\n\r", obj->value[2]);
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        ptc(ch, 
                  "[v0] Уровень:          [%d]\n\r"
                  "[v1] Всего зарядов:    [%d]\n\r"
                  "[v2] Осталось зарядов: [%d]\n\r"
                  "[v3] Заклинание:       %s {D(? spells){x\n\r",
                  obj->value[0],
                  obj->value[1],
                  obj->value[2],
                  get_skill_name( obj->value[3] ));
        break;

    case ITEM_PORTAL:
        ptc(ch, 
                  "[v0] Заряды:         [%d]\n\r"
                  "[v1] Флаги выхода:   %s {D(? exit_flags){x\n\r"
                  "[v2] Флаги портала:  %s {D(? portal_flags){x\n\r"
                  "[v3] Ведет в (внум): [%d]\n\r"
                  "[v4] Ключ (внум):    [%d]\n\r",
                  obj->value[0],
                  web_flag_menu(showWeb, exit_flags, "v1", exit_flags.names(obj->value[1]), OLC_FRAME_NONE).c_str(),
                  web_flag_menu(showWeb, portal_flags, "v2", portal_flags.names(obj->value[2]), OLC_FRAME_NONE).c_str(),
                  obj->value[3], obj->value[4]);
        break;

    case ITEM_FURNITURE:
        ptc(ch, 
                  "[v0] Макс. людей:     [%d]\n\r"
                  "[v1] Макс. вес:       [%d]\n\r"
                  "[v2] Флаги мебели:    %s {D(? furniture_flags){x\n\r"
                  "[v3] Бонус здоровья:  [%d]\n\r"
                  "[v4] Бонус маны:      [%d]\n\r",
                  obj->value[0],
                  obj->value[1],
                  web_flag_menu(showWeb, furniture_flags, "v2", furniture_flags.names(obj->value[2]), OLC_FRAME_NONE).c_str(),
                  obj->value[3],
                  obj->value[4]);
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        ptc(ch, 
                  "[v0] Уровень:     [%d]\n\r"
                  "[v1] Заклинание:  %s {D(? spells){x\n\r"
                  "[v2] Заклинание:  %s\n\r"
                  "[v3] Заклинание:  %s\n\r"
                  "[v4] Заклинание:  %s\n\r",
                  obj->value[0],
                  get_skill_name( obj->value[1] ),
                  get_skill_name( obj->value[2] ),
                  get_skill_name( obj->value[3] ),
                  get_skill_name( obj->value[4] ) );
        break;

    case ITEM_ARMOR:
        ptc(ch, 
                  "[v0] AC колющее      [%d]\n\r"
                  "[v1] AC удар         [%d]\n\r"
                  "[v2] AC режущее      [%d]\n\r"
                  "[v3] AC экзотика     [%d]\n\r",
                  obj->value[0],
                  obj->value[1],
                  obj->value[2],
                  obj->value[3]);
        break;

    case ITEM_WEAPON:
        random = item_is_random(obj);

        ptc(ch, "[v0] Вид оружия:   %s {D(? weapon_class){x\n\r",
            web_flag_menu(showWeb, weapon_class, "v0", weapon_class.name(obj->value[0]), OLC_FRAME_NONE).c_str());
            
        if (!random) {
            ptc(ch, "[v1] Число бросков кубика: [%d]\n\r", obj->value[1]);
            ptc(ch, "[v2] Число граней кубика : [%d] {D(повреждения %dd%d, среднее %d = (v2+1)*v1/2){x\r\n", 
                        obj->value[2], obj->value[1], obj->value[2], weapon_ave(obj));
        }

        ptc(ch, "[v3] Тип удара:    %s {D(? weapon_flags){x\n\r",
            web_flag_menu(showWeb, weapon_flags, "v3", weapon_flags.name(obj->value[3]), OLC_FRAME_NONE).c_str());

        if (!random) {
            ptc(ch, "[v4] Флаги оружия: %s {D(? weapon_type2){x\n\r",
                web_flag_menu(showWeb, weapon_type2, "v4", weapon_type2.names(obj->value[4]), OLC_FRAME_NONE).c_str());
        }
        break;

    case ITEM_CONTAINER:
        pKey = obj->value[2] > 0 ? get_obj_index(obj->value[2]) : 0;
        ch->pecho(
            "[v0] Вместимость:[%d фунтов]\n\r"
            "[v1] Флаги:      %s {D(? container_flags){x\n\r"
            "[v2] Ключ:       [%d] %N1\n\r"
            "[v3] Макс. вес:  [%d фунтов]\n\r"
            "[v4] Коэф. снижения веса: [%d]\n\r",
            obj->value[0],
            web_flag_menu(showWeb, container_flags, "v1", container_flags.names(obj->value[1]), OLC_FRAME_PLAIN).c_str(),
            obj->value[2],
            pKey ? pKey->getShortDescr(LANG_DEFAULT) : "нет",
            obj->value[3],
            obj->value[4]);
        break;

    case ITEM_DRINK_CON:
        ptc(ch,
            "[v0] Всего жидкости:  [%d]\n\r"
            "[v1] Налито жидкости: [%d]\n\r"
            "[v2] Тип жидкости:     %s (%s) {D(? liquid){x\n\r"
            "[v3] Флаги:            %s {D(? drink_flags){x\n\r"
            "[v4] Пробка:           %d\n\r",
            obj->value[0],
            obj->value[1],
            web_registry_menu(showWeb, liquidManager, "v2", liquidManager->find( obj->value[2] )->getName( ), "none", OLC_FRAME_NONE).c_str( ),
            liquidManager->find( obj->value[2] )->getShortDescr( ).ruscase( '1' ).c_str( ),
            web_flag_menu(showWeb, drink_flags, "v3", drink_flags.names(obj->value[3]), OLC_FRAME_NONE).c_str( ),
            obj->value[4]);
        break;

    case ITEM_FOUNTAIN:
        ptc(ch, 
            "[v0] Всего жидкости:  [%d]\n\r"
            "[v1] Налито жидкости: [%d]\n\r"
            "[v2] Тип жидкости:    %s (%s) {D(? liquid){x\n\r"
            "[v3] Макс. людей:     [%d]\n\r"
            "[v4] Флаги мебели:    %s {D(? furniture_flags){x\n\r",
            obj->value[0],
            obj->value[1],
            web_registry_menu(showWeb, liquidManager, "v2", liquidManager->find( obj->value[2] )->getName( ), "none", OLC_FRAME_NONE).c_str( ),
            liquidManager->find( obj->value[2] )->getShortDescr( ).ruscase( '1' ).c_str( ),
            obj->value[3],
            web_flag_menu(showWeb, furniture_flags, "v4", furniture_flags.names(obj->value[4]), OLC_FRAME_NONE).c_str());
        break;

    case ITEM_FOOD:
        ptc(ch,
            "[v0] Сытость, часов:    [%d]\n\r"
            "[v1] Насыщение, часов:  [%d]\n\r"
            "[v3] Отравлено:         %s\n\r",
            obj->value[0],
            obj->value[1],
            web_bool_menu(showWeb, "v3", obj->value[3] != 0 ? "да" : "нет", OLC_FRAME_NONE).c_str());
        break;

    case ITEM_MONEY:
        ptc(ch, "[v0] Серебро:  [%d]\n\r", obj->value[0]);
        ptc(ch, "[v1] Золото:   [%d]\n\r", obj->value[1]);
        break;

    }
}

bool set_obj_values(Character * ch, OBJ_INDEX_DATA * pObj, int value_num, const char *argument)
{
    Liquid *liq;

    switch (pObj->item_type) {
    default:
        stc("Это значение не используется.\n\r", ch);
        return false;
   
   case ITEM_SPELLBOOK:
   case ITEM_TEXTBOOK:
        switch (value_num) {
        default:
            stc("Это значение не используется.\r\n", ch);
            return false;
        case 0:
            stc("Число страниц установлено.\r\n", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Число занятых страниц установлено.\r\n", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("Макс. качество установлено.\r\n", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
        case 4:
            stc("Изучаемое умение или заклинание установлено.\r\n", ch);
            pObj->value[value_num] = skill_lookup(argument);
            break;
        }
        break;
       
    case ITEM_RECIPE:
        switch (value_num) {
            int value;
        default:
            stc("Это значение не используется.\r\n", ch);
            return false;
        case 0:
            if ((value = recipe_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[0], value);
            else {
                show_help(ch, "recipe");
                return false;
            }
            stc("Флаги рецепта переключены.\n\r", ch);
            break;
        case 2:
            stc("Сложность рецепта установлена.\r\n", ch);
            pObj->value[2] = atoi(argument);
            break;
        }
        break;
 
    case ITEM_MAP:
        switch (value_num) {
            int i;

        default:
            stc("Это значение не используется.\n\r", ch);
            return false;
        case 0: 
            i = atoi( argument );
            if (i)
                stc("Не исчезает в руках.\n\r", ch);
            else 
                stc("Исчезает в руках.\n\r", ch);

            pObj->value[0] = i;
            break;
        }
        break;
    case ITEM_KEYRING: 
        switch (value_num) {
        default:
            stc("Это значение не используется.\n\r", ch);
            return false;
        case 0: 
            pObj->value[0] = atoi( argument );
            ptc(ch, "На кольцо поместится не больше %d ключей.\n\r", pObj->value[0]);
        }
        break;
    case ITEM_LOCKPICK: 
        switch (value_num) {
        int i;

        default:
            stc("Это значение не используется.\n\r", ch);
            return false;
        case 0: 
            i = atoi( argument );
            
            if (i == Keyhole::LOCK_VALUE_MULTI)
                stc("Отмычка откроет замок любого типа.\n\r", ch);
            else if (i == Keyhole::LOCK_VALUE_BLANK)
                stc("Это будет заготовка для отмычки.\n\r", ch);
            else if (i < 0 || i >= Keyhole::MAX_KEY_TYPES) {
                stc("Неверный тип замка.\n\r", ch);
                return false;
            }
            else
                ptc(ch, "Отмычка откроет замки типа %d.\n\r", i);

            pObj->value[0] = i;
            break;
        case 1: 
            i = atoi( argument );
            if (i < 0 || i > 100) {
                stc("Неверное качество, допустимо 0..100.\n\r", ch);
                return false;
            }
            
            ptc(ch, "Качество отмычки: %d%%.\n\r", i);
            pObj->value[1] = i;
            break;
        }
        break;
    case ITEM_KEY: 
        switch (value_num) {
        int i;

        default:
            stc("Это значение не используется.\n\r", ch);
            return false;
        case 0: 
            i = atoi( argument );
            if (i) {
                if (i == 1)
                    stc("Ключ не исчезает в руках.\n\r", ch);
                else
                    ptc(ch, "Ключ продержится в руках %d минут.\r\n", i);
            }
            else 
                stc("Ключ исчезает в руках.\n\r", ch);

            pObj->value[0] = i;
            break;
        case 1: 
            i = atoi( argument );
            if (i)
                stc("Ключ исчезает на земле.\n\r", ch);
            else 
                stc("Ключ не исчезает на земле.\n\r", ch);

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
            stc("Часы света установлены.\n\r", ch);
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
            stc("Уровень заклинания установлен.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Всего зарядов установлено.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("Осталось зарядов установлено.\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("Заклинание установлено.\n\r", ch);
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
            stc("Уровень заклинания установлен.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Заклинание 1 установлено.\n\r", ch);
            pObj->value[1] = skill_lookup(argument);
            break;
        case 2:
            stc("Заклинание 2 установлено.\n\r", ch);
            pObj->value[2] = skill_lookup(argument);
            break;
        case 3:
            stc("Заклинание 3 установлено.\n\r", ch);
            pObj->value[3] = skill_lookup(argument);
            break;
        case 4:
            stc("Заклинание 4 установлено.\n\r", ch);
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
            stc("AC против колющего установлен.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("AC против удара установлен.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("AC против режущего установлен.\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("AC против экзотики установлен.\n\r", ch);
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
            stc("Вид оружия установлен.\n\r", ch);
            pObj->value[0] = weapon_class.value( argument );
            break;
        case 1:
            stc("Число бросков кубика установлено.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            stc("Число граней кубика установлено.\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("Тип удара установлен.\n\r", ch);
            pObj->value[3] = weapon_flags.value( argument );
            break;
        case 4:
            if ((value = weapon_type2.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[value_num], value);
            else {
                show_help(ch, "weapon_type2");
                return false;
            }
            stc("Флаги оружия переключены.\n\r", ch);
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
            stc("Заряды установлены.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            if ((value = exit_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[value_num], value);
            else {
                show_help(ch, "exit");
                return false;
            }
            stc("Флаги выхода переключены.\n\r", ch);
            break;
        case 2:
            if ((value = portal_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[2], value);
            else {
                show_help(ch, "portal");
                return false;
            }
            stc("Флаги портала переключены.\n\r", ch);
            break;
        case 3:
            stc("Внум выхода установлен.\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            stc("Внум ключа установлен.\n\r", ch);
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
            stc("Макс. людей установлено.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Макс. вес установлен.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            if ((value = furniture_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[2], value);
            else {
                show_help(ch, "furniture");
                return false;
            }
            stc("Флаги мебели переключены.\n\r", ch);
            break;
        case 3:
            stc("Бонус здоровья установлен.\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            stc("Бонус маны установлен.\n\r", ch);
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
            stc("Вместимость установлена.\n\r", ch);
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
            stc("Флаги контейнера переключены.\n\r", ch);
            break;
        case 2:
            if (atoi(argument) > 0) {
                if (!get_obj_index(atoi(argument))) {
                    stc("Такого предмета нет.\n\r", ch);
                    return false;
                }
                if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
                    stc("Этот предмет не ключ.\n\r", ch);
                    return false;
                }
            }
            stc("Ключ контейнера установлен.\n\r", ch);
            pObj->value[2] = atoi(argument);
            break;
        case 3:
            stc("Макс. вес контейнера установлен.\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            stc("Коэф. снижения веса установлен.\n\r", ch);
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
            stc("Установлено максимальное количество жидкости.\r\n", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Установлено текущее количество жидкости.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            if (!( liq = liquidManager->findExisting( argument ) )) {
                stc("Нет такой жидкости, смотри '? liquid'.\r\n", ch);
                return false;
            }
            pObj->value[2] = liq->getIndex( );
            stc("Тип жидкости установлен.\n\r", ch);
            break;
        case 3:
            if ((value = drink_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[3], value);
            else {
                show_help(ch, "drink");
                return false;
            }
            stc("Флаги сосуда переключены.\n\r", ch);
            break;
        case 4:
            stc("Внум пробки установлен.\n\r", ch);
            pObj->value[4] = atoi(argument);
            break;
        }
        break;
    case ITEM_FOUNTAIN:
        switch (value_num) {
            int value;

        default:
//            do_help(ch, "ITEM_FOUNTAIN");
            return false;
        case 0:
            stc("Установлено максимальное количество жидкости.\r\n", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Установлено текущее количество жидкости.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 2:
            if (!( liq = liquidManager->findExisting( argument ) )) {
                stc("Нет такой жидкости, смотри '? liquid'.\r\n", ch);
                return false;
            }
            pObj->value[2] = liq->getIndex( );
            stc("Тип жидкости установлен.\n\r", ch);
            break;
        case 3:
            stc("Макс. людей установлено.\n\r", ch);
            pObj->value[3] = atoi(argument);
            break;
        case 4:
            if ((value = furniture_flags.bitstring( argument )) != NO_FLAG)
                TOGGLE_BIT(pObj->value[4], value);
            else {
                show_help(ch, "furniture");
                return false;
            }
            stc("Флаги мебели переключены.\n\r", ch);
            break;
        }
        break;
    case ITEM_FOOD:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_FOOD");
            return false;
        case 0:
            stc("Сытость установлена.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Насыщение установлено.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        case 3:
            // y/n sets it (the web menu sends these), anything else toggles.
            if (arg_is_yes(argument)) {
                pObj->value[3] = 1;
                stc("Еда отравлена.\n\r", ch);
            } else if (arg_is_no(argument)) {
                pObj->value[3] = 0;
                stc("Еда не отравлена.\n\r", ch);
            } else {
                pObj->value[3] = (pObj->value[3] == 0) ? 1 : 0;
                stc("Отравление переключено.\n\r", ch);
            }
            break;
        }
        break;
    case ITEM_MONEY:
        switch (value_num) {
        default:
//            do_help(ch, "ITEM_MONEY");
            return false;
        case 0:
            stc("Серебро установлено.\n\r", ch);
            pObj->value[0] = atoi(argument);
            break;
        case 1:
            stc("Золото установлено.\n\r", ch);
            pObj->value[1] = atoi(argument);
            break;
        }
        break;
    }
    show_obj_values(ch, pObj, true);
    return true;
}


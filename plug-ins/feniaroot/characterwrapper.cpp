/* $Id: characterwrapper.cpp,v 1.1.4.50.4.40 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include <iostream>
#include <jsoncpp/json/json.h>

#include "logstream.h"
#include "mobilebehaviormanager.h"
#include "basicmobilebehavior.h"

#include "skill.h"
#include "skillmanager.h"
#include "spelltarget.h"
#include "clan.h"
#include "behavior.h"
#include "affect.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "room.h"

#include "xmlattributetrust.h"
#include "xmlkillingattribute.h"
#include "fight_exception.h"
#include "subprofession.h"
#include "screenreader.h"
#include "profflags.h"
#include "occupations.h"
#include "follow_utils.h"
#include "interp.h"
#include "comm.h"
#include "save.h"

#include "fight.h"
#include "skill_utils.h"
#include "immunity.h"
#include "magic.h"
#include "movement.h"
#include "act_move.h"
#include "merc.h"
#include "../anatolia/handler.h"
#include "alignment.h"
#include "wiznet.h"
#include "xmlattributecoder.h"
#include "xmlattributerestring.h"
#include "commonattributes.h"
#include "pet.h"
#include "recipeflags.h"
#include "damageflags.h"
#include "act.h"
#include "selfrate.h"
#include "religionutils.h"
#include "websocketrpc.h"

#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "mobindexwrapper.h"
#include "structwrappers.h"
#include "affectwrapper.h"
#include "xmleditorinputhandler.h"
#include "reglist.h"
#include "regcontainer.h"
#include "nativeext.h"
#include "idcontainer.h"
#include "fenia_utils.h"
#include "wrap_utils.h"
#include "subr.h"
#include "def.h"

GSN(dark_shroud);
GSN(manacles);
GSN(charm_person);
DESIRE(hunger);
DESIRE(bloodlust);
DESIRE(thirst);
DESIRE(full);
DESIRE(drunk);

void password_set( PCMemoryInterface *pci, const DLString &plainText );
const char *ttype_name( int ttype );
DLString regfmt(Character *to, const RegisterList &argv);
list< ::Object *> get_objs_list_type( Character *ch, int type, ::Object *list );
void obj_from_anywhere( ::Object *obj );
void do_visible( Character * );

using namespace std;
using namespace Scripting;
using Scripting::NativeTraits;

NMI_INIT(CharacterWrapper, "персонаж (моб или игрок)")

CharacterWrapper::CharacterWrapper( ) : target( NULL )
{
}

void CharacterWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );
    
    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void CharacterWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        if (Scripting::gc)
            LogStream::sendError() << "Character wrapper: extract without target" << endl;
    }
    
    GutsContainer::extract( count );
}

void CharacterWrapper::setTarget( ::Character *target )
{
    this->target = target;
    id = target->getID( );
}

void CharacterWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "Character is dead" );

    if (target == NULL) 
        throw Scripting::Exception( "Character is offline" );
}

Character * CharacterWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

/*
 * FIELDS
 */
NMI_GET( CharacterWrapper, id, "уникальный идентификатор персонажа" )
{
    return Register( DLString(id) );
}

NMI_GET( CharacterWrapper, online, "true, если персонаж в мире" )
{
    return Register( target != NULL );
}

NMI_GET( CharacterWrapper, dead, "true, если персонажа уничтожили или моб только что умер" )
{
    if (zombie)
        return true;

    if (!target)
        return true;        

    checkTarget();
    return target->isDead();
}

NMI_SET( CharacterWrapper, dead, "true, если персонажа уничтожили или моб только что умер" )
{
    checkTarget( );
    target->setDead();
}


#define CHK_PC \
    if (!target->is_npc()) \
        throw Scripting::Exception( "NPC field requested on PC" ); 
#define CHK_NPC \
    if (target->is_npc()) \
        throw Scripting::Exception( "PC field requested on NPC" ); 

#define GETWRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    return wrap(target->x); \
}
#define GET_NPC_WRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    CHK_PC \
    return wrap(target->getNPC()->x); \
}
#define GET_PC_WRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    CHK_NPC \
    return wrap(target->getPC()->x); \
}

GET_NPC_WRAP( pIndexData, "структура с прототипом для всех мобов с данным vnum"
                          "(mob index data, т.е. то, редактируется с помощью OLC)")
GETWRAP( reply, "чар, который последний говорил с нами. по команде reply реплика отправится именно ему" )
GETWRAP( next, "следующий чар в глобальном списке всех чаров, .char_list" )
GETWRAP( next_in_room, "следующий чар в этой комнате, в списке people у комнаты" )
GETWRAP( master, "тот, за кем следуем" )
GETWRAP( leader, "лидер группы или тот, кто очаровал" )
GETWRAP( fighting, "тот, с кем сражаемся" )
GETWRAP( last_fought, "чар, с которым сражались последний раз" )
GET_PC_WRAP( pet, "моб, домашнее животное" )
GET_PC_WRAP( switchedTo, "в какого моба вселились" )
GETWRAP( doppel, "игрок, которому подражаем с помощью doppelganger. "
                 "для зеркал - игрок, который их создал" )
GET_PC_WRAP( guarding, "игрок, которого охраняем с помощью умения guard" )
GET_PC_WRAP( guarded_by, "игрок, который нас охраняет" )

GETWRAP( carrying, "первый объект в списке инвентаря/экипировки")
NMI_GET( CharacterWrapper, inventory, "список всех предметов в инвентаре" )
{
    RegList::Pointer rc( NEW );
    checkTarget( );
    
    for (::Object *obj = target->carrying; obj != 0; obj = obj->next_content)  
    if (obj->wear_loc == wear_none)
        rc->push_back(wrap(obj));

    return wrap(rc);
}

NMI_GET( CharacterWrapper, equipment, "список всех предметов в экипировке" )
{
    RegList::Pointer rc( NEW );
    checkTarget( );
    
    for (::Object *obj = target->carrying; obj != 0; obj = obj->next_content)  
    if (obj->wear_loc != wear_none)
            rc->push_back(wrap(obj));

    return wrap(rc);
}

NMI_GET( CharacterWrapper, items, "список всех предметов в инвентаре или экипировке" )
{
    RegList::Pointer rc( NEW );
    checkTarget( );
    
    for (::Object *obj = target->carrying; obj != 0; obj = obj->next_content)  
        rc->push_back(wrap(obj));

    return wrap(rc);
}


GETWRAP( on, "объект, мебель, на которой сидим" )

GETWRAP( in_room, "комната, в которой сейчас находимся" ) 
GETWRAP( was_in_room, "комната, в которой находились перед закапыванием в могилу")
GETWRAP( mount, "на ком мы верхом или кто верхом на нас" )
    
NMI_SET( CharacterWrapper, mount, "лидер группы или тот, кто очаровал" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
        target->mount = NULL;
    else
        target->mount = arg2character( arg );
}

NMI_SET( CharacterWrapper, leader, "лидер группы или тот, кто очаровал" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
        target->leader = NULL;
    else
        target->leader = arg2character( arg );
}
NMI_SET( CharacterWrapper, last_fought, "чар, с которым сражались последний раз" )
{
    checkTarget( );

    if (arg.type == Register::NONE)
        target->last_fought = NULL;
    else
        target->last_fought = arg2character( arg );
}


#define ARMOR(x) \
NMI_GET( CharacterWrapper, armor##x, "класс брони" ) \
{ \
    checkTarget(); \
    return target->armor[x]; \
} \
NMI_SET( CharacterWrapper, armor##x, "класс брони" ) \
{ \
    checkTarget(); \
    target->armor[x] = arg.toNumber(); \
}

ARMOR(0)
ARMOR(1)
ARMOR(2)
ARMOR(3)
#undef ARMOR

NMI_GET( CharacterWrapper, pc, "экземпляр игрока" )
{
    checkTarget( );
    return wrap(target->getPC( ));
}

NMI_GET( CharacterWrapper, logon, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    return (int)target->getPC( )->age.getLogon( ).getTime( );
}
NMI_SET( CharacterWrapper, logon, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->age.setLogon( arg.toNumber( ) );
}
NMI_GET( CharacterWrapper, terminal_type, "тип терминала у mud-клиента" )
{
    checkTarget( );
    CHK_NPC
    if (!target->desc)
        return "";
    return ttype_name( target->desc->telnet.ttype );
}

NMI_GET( CharacterWrapper, webclient, "true если использует вебклиент" )
{
    checkTarget( );
    CHK_NPC
    if (!target->desc)
        return false;
    return is_websock(target);
}

NMI_GET( CharacterWrapper, attack_name, "англ название типа атаки (таблица в коде attack_table)")
{
    checkTarget();
    return attack_table[target->dam_type].name;
}

NMI_GET( CharacterWrapper, attack_noun, "русск название типа атаки (таблица в коде attack_table)")
{
    checkTarget();
    return attack_table[target->dam_type].noun;
}

NMI_GET( CharacterWrapper, attack_damage, "название типа повреждения (таблица .tables.damage_table)")
{
    checkTarget();
    
    return damage_table.name(attack_table[target->dam_type].damage);
}

NMI_SET( CharacterWrapper, damage_number, "повреждения моба: сколько раз кидать кубик" )
{
    checkTarget( );
    CHK_PC
    target->getNPC( )->damage[DICE_NUMBER] = arg.toNumber();        
}
NMI_SET( CharacterWrapper, damage_type, "повреждения моба: кол-во граней кубика" )
{
    checkTarget( );
    CHK_PC
    target->getNPC( )->damage[DICE_TYPE] = arg.toNumber();        
}
NMI_GET( CharacterWrapper, damage_number, "повреждения моба: сколько раз кидать кубик" )
{
    checkTarget( );
    CHK_PC
    return target->getNPC( )->damage[DICE_NUMBER];
}
NMI_GET( CharacterWrapper, damage_type, "повреждения моба: кол-во граней кубика" )
{
    checkTarget( );
    CHK_PC
    return target->getNPC( )->damage[DICE_TYPE];
}

NMI_INVOKE( CharacterWrapper, setLevel, "(level): установить уровень мобу" )
{
    checkTarget();
    CHK_PC

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    target->setLevel( args.front( ).toNumber( ) );
    return Register( );
}

NMI_GET( CharacterWrapper, short_descr, "короткое описание моба" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getShortDescr( ) );
}

NMI_SET( CharacterWrapper, short_descr, "короткое описание моба" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setShortDescr( arg.toString( ) );
}

NMI_GET( CharacterWrapper, long_descr, "длинное описание моба" )
{
    checkTarget( );
    CHK_PC
    return Register( target->getNPC()->getLongDescr( ) );
}

NMI_SET( CharacterWrapper, long_descr, "длинное описание моба" )
{
    checkTarget( );
    CHK_PC
    target->getNPC()->setLongDescr( arg.toString( ) );
}

NMI_GET( CharacterWrapper, description, "то что видно по look mob" )
{
    checkTarget( );
    return Register( target->getDescription( ) );
}

NMI_SET( CharacterWrapper, description, "то что видно по look mob" )
{
    checkTarget( );
    target->setDescription( arg.toString( ) );
}

NMI_GET( CharacterWrapper, spec_fun, "спец-процедура") 
{
    checkTarget( ); 
    CHK_PC
    if (target->getNPC()->spec_fun.func)
        return Register( spec_name(target->getNPC()->spec_fun.func) );
    else
        return Register( );
}

NMI_SET( CharacterWrapper, spec_fun, "спец-процедура") 
{
    checkTarget( ); 
    CHK_PC

    SPEC_FUN *spec;

    if (arg.type == Register::NONE)
        target->getNPC()->spec_fun.clear();
    else if ((spec = spec_lookup(arg.toString().c_str())) == 0)
        throw Scripting::Exception("Unknown spec function name");
    else
        target->getNPC()->spec_fun = spec;
}

NMI_GET(CharacterWrapper, questpoints, "qp")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getQuestPoints();
}
NMI_SET(CharacterWrapper, questpoints, "qp")
{
    checkTarget();
    CHK_NPC
    target->getPC()->setQuestPoints(arg.toNumber());
}
NMI_GET( CharacterWrapper, trust, "уровень привилегий" )
{
    PCMemoryInterface *pci;
    
    checkTarget( );
    
    if (!target->is_npc( ) && target->getLevel( ) == 0) { // may be not loaded yet
        if (( pci = PCharacterManager::find( target->getName( ) ) ))
            return pci->get_trust( );
        else
            return 0;
    }

    return target->get_trust( );
}

NMI_GET( CharacterWrapper, pretitle, "претитул" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getPretitle( ) );
}

NMI_SET( CharacterWrapper, pretitle, "претитул" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setPretitle( arg.toString( ) );
}

NMI_GET( CharacterWrapper, title, "титул" )
{
    checkTarget( );
    CHK_NPC
    return Register( target->getPC( )->getTitle( ) );
}

NMI_SET( CharacterWrapper, title, "титул" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setTitle( arg.toString( ) );
}

NMI_GET( CharacterWrapper, password, "пароль: deprecated" )
{
    PCMemoryInterface *pci;

    checkTarget( );
    CHK_NPC

    if (( pci = PCharacterManager::find( target->getName( ) ) ))
        return pci->getPassword( );
    else
        return target->getPC( )->getPassword( );
}

NMI_SET( CharacterWrapper, password, "пароль" )
{
    checkTarget( );
    CHK_NPC
    password_set( target->getPC( ), arg.toString( ) );
}

NMI_GET( CharacterWrapper, remort_count, "кол-во ремортов" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getRemorts( ).size( ) );
}

NMI_GET( CharacterWrapper, altar, "vnum комнаты-алтаря в родном городе персонажа" )
{
    checkTarget( );
    CHK_NPC
    return Register( (int)target->getPC( )->getHometown( )->getAltar( ) );
}


NMI_GET( CharacterWrapper, craftProfessions, "map из названия->уровень мастерства для дополнительных профессий" )
{
    ::Pointer<RegContainer> rc(NEW);
    checkTarget( );
    CHK_NPC
    list<CraftProfession::Pointer>::const_iterator p;
    list<CraftProfession::Pointer> profs = craftProfessionManager->getProfessions();
     
    for (p = profs.begin(); p != profs.end(); p++)
        (*rc)->map[(*p)->getName()] = Register((*p)->getLevel(target->getPC()));

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

#define CONDITION(type, api) \
NMI_GET( CharacterWrapper, cond_##type, api ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    return Register( target->getPC( )->desires[desire_##type] ); \
} \
NMI_SET( CharacterWrapper, cond_##type, "изменить '" api "' на указанное число баллов" ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    desire_##type->gain( target->getPC( ), arg.toNumber( ) ); \
}

CONDITION(hunger,    "голод");
CONDITION(thirst,    "жажда");
CONDITION(full,      "заполненность желудка");
CONDITION(bloodlust, "жажда крови");
CONDITION(drunk,     "опьянение");
#undef CONDITION


NMI_SET( CharacterWrapper, sex, "пол (таблица .tables.sex_table)")
{
    checkTarget( );
    target->setSex( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, sex, "пол (таблица .tables.sex_table)")
{
    checkTarget( );
    return target->getSex( );
}

NMI_GET( CharacterWrapper, wait, "wait state (в пульсах, 1 пульс = четверть секунды)")
{
    checkTarget( );
    return target->wait;
}

NMI_SET( CharacterWrapper, wait, "wait state (в пульсах, 1 пульс = четверть секунды)")
{
    checkTarget( );
    target->setWait( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, boat, "объект лодки" )
{
    checkTarget( );
    return wrap( boat_object_find( target ) );
}

NMI_GET( CharacterWrapper, slow, "true если есть бит slow, нету хасты и (в случае мобов) бита fast")
{
    checkTarget();
    return IS_SLOW(target);
}

NMI_GET( CharacterWrapper, quick, "true если есть бит haste/fast и нету slow")
{
    checkTarget();
    return IS_QUICK(target);
}

NMI_GET( CharacterWrapper, flying, "true если мы GHOST, летаем или верхом на летающем скакуне" )
{
    checkTarget( );
    
    if (IS_GHOST(target))
        return true;
        
    if (is_flying( target ))
        return true;

    if (MOUNTED(target) && is_flying(MOUNTED(target)))
        return true;

    return false;
}

NMI_INVOKE( CharacterWrapper, flydown, "опуститься на землю без задержек, вернет true если до этого летали" )
{
    checkTarget();

    if (is_flying(target)) {
        target->posFlags.setBit( POS_FLY_DOWN );
        target->pecho( "Ты перестаешь летать." );
        target->recho( "%^C1 перестает летать.", target ); 
        return Register(true);
    }

    return Register(false);
}

NMI_GET( CharacterWrapper, ambushing, "строка, на кого сидим в засаде" )
{
    checkTarget( );
    return Register(target->ambushing);
}

NMI_SET( CharacterWrapper, ambushing, "строка, на кого сидим в засаде" )
{
    checkTarget();
    DLString str = arg2string(arg);

    if (str.empty()) {
        free_string(target->ambushing);
        target->ambushing = &str_empty[0]; 
    } else {
        target->ambushing = str_dup(str.c_str());
    }
}

NMI_GET( CharacterWrapper, neutral, "true если персонаж нейтральный" )
{
    checkTarget( );
    return IS_NEUTRAL(target);
}

NMI_GET( CharacterWrapper, evil, "true если персонаж злой" )
{
    checkTarget( );
    return IS_EVIL(target);
}

NMI_GET( CharacterWrapper, good, "true если персонаж добрый" )
{
    checkTarget( );
    return IS_GOOD(target);
}

NMI_GET( CharacterWrapper, alignName, "название натуры" )
{
    checkTarget( );
    return align_name( target );
}

NMI_GET( CharacterWrapper, mod_beats, "на сколько процентов увеличены или уменьшены задержки от умений" )
{
    checkTarget( );
    return target->mod_beats;
}

#define DEF_STAT(x, stat, help) \
NMI_GET( CharacterWrapper, cur_##x, "текущий параметр: " help ) \
{ \
    checkTarget( ); \
    return Register( target->getCurrStat(stat) ); \
} \
NMI_GET( CharacterWrapper, max_train_##x, "максимум тренировки для параметра: " help ) \
{ \
    checkTarget( ); \
    CHK_NPC \
    return Register( target->getPC( )->getMaxTrain(stat) ); \
} \
NMI_GET( CharacterWrapper, perm_##x, "перманентный параметр: " help ) \
{ \
    checkTarget( ); \
    return Register( target->perm_stat[stat] ); \
} \
NMI_SET( CharacterWrapper, perm_##x, "перманентный параметр: " help ) \
{ \
    checkTarget( ); \
    int max_value = (target->is_npc( ) ? MAX_STAT : target->getPC( )->getMaxTrain(stat)); \
    target->perm_stat[stat] = URANGE(1, arg.toNumber( ), max_value); \
}

DEF_STAT(str, STAT_STR, "сила")
DEF_STAT(int, STAT_INT, "ум")
DEF_STAT(wis, STAT_WIS, "мудрость")
DEF_STAT(dex, STAT_DEX, "ловкость")
DEF_STAT(con, STAT_CON, "телосложение")
DEF_STAT(cha, STAT_CHA, "харизма")

#define STR_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x = arg.toString(); \
}

STR_FIELD(prompt, "строка состояния")
STR_FIELD(batle_prompt, "строка состояния в бою")

#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( (int) target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x = arg.toNumber(); \
}

#define FLAG_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    return Register( (int) target->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    checkTarget( ); \
    target->x.setValue( arg.toNumber() ); \
}

INT_FIELD(ethos, "добропорядочность")
INT_FIELD(timer, "сколько минут прошло с последней команды")
INT_FIELD(daze, "daze state (в пульсах, 1 пульс = четверть секунды)")
INT_FIELD(hit, "текущее здоровье (hit points)")
INT_FIELD(max_hit, "максимальное здоровье")
INT_FIELD(mana, "текущая mana")
INT_FIELD(max_mana, "максимальная mana")
INT_FIELD(move, "текущие moves")
INT_FIELD(max_move, "максимальные moves")
INT_FIELD(gold, "золото")
INT_FIELD(silver, "серебро")
INT_FIELD(exp, "суммарный опыт")
INT_FIELD(invis_level, "уровень для wisinvis")
INT_FIELD(incog_level, "уровень для incognito")
INT_FIELD(lines, "кол-во строк в буфере вывода")
INT_FIELD(act, "act флаги для мобов и plr для игроков (таблицы .tables.act_flags и plr_flags)")
INT_FIELD(comm, "comm флаги (таблица .tables.comm_flags)")
INT_FIELD(add_comm, "расширение поля comm (таблица .tables.add_comm_flags)")
INT_FIELD(imm_flags, "флаги иммунитета (таблица .tables.imm_flags)")
INT_FIELD(res_flags, "флаги сопротивляемости (таблица .tables.res_flags)")
INT_FIELD(vuln_flags, "флаги уязвимости (таблица .tables.res_flags)")
INT_FIELD(affected_by, "флаги аффектов (таблица .tables.affect_flags)")
INT_FIELD(detection, "флаги детектов (таблица .tables.detect_flags)")
INT_FIELD(position, "позиция (таблица .tables.position_table)")
INT_FIELD(posFlags, "флаги позиции (таблица .tables.position_flags)")
INT_FIELD(carry_number, "количество вещей которое несет чар")
INT_FIELD(saving_throw, "савесы")
INT_FIELD(alignment, "натура, от -1000 до 1000")
INT_FIELD(hitroll, "точность")
INT_FIELD(damroll, "урон")
INT_FIELD(wimpy, "трусость. при скольки hp чар будет убегать автоматически")
INT_FIELD(dam_type, "тип повреждения (таблица .tables.weapon_flags)")
INT_FIELD(form, "форма тела (таблица .tables.form_flags)")
INT_FIELD(parts, "части тела (таблица .tables.part_flags)")
INT_FIELD(size, "размер (таблица .tables.size_table)")
INT_FIELD(death_ground_delay, "счетчик ловушки")
FLAG_FIELD(trap, "флаги ловушки (таблица .tables.trap_flags)")
INT_FIELD(riding, "если mount!=null: true - мы верхом, false - мы оседланы")

#undef INT_FIELD
#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    CHK_NPC \
    checkTarget( ); \
    return Register( (int) target->getPC()->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    CHK_NPC \
    checkTarget( ); \
    target->getPC()->x = arg.toNumber(); \
}

INT_FIELD(last_level, "какой был played, когда набили последний левел")
INT_FIELD(last_death_time, "когда последний раз был убит")
INT_FIELD(ghost_time, "сколько висит ghost")
INT_FIELD(PK_time_v, "сколько висит violent")
INT_FIELD(PK_time_sk, "сколько висит slain и killer")
INT_FIELD(PK_time_t, "сколько висит thief")
INT_FIELD(PK_flag, "KILLER, SLAIN, VIOLENT, GHOST, THIEF")
INT_FIELD(death, "сколько раз умирал")
INT_FIELD(perm_hit, "max hp без шмота")
INT_FIELD(perm_mana, "max mana без шмота")
INT_FIELD(perm_move, "max move без шмота")
INT_FIELD(practice, "сколько практик")
INT_FIELD(train, "сколько тренировок")
INT_FIELD(bank_s, "серебра в банке")
INT_FIELD(bank_g, "золота в банке")
INT_FIELD(config, "настройки чара (таблица .tables.config_flags)")
INT_FIELD(shadow, "сколько висеть тени (shadowlife) в секундах")
    
#undef INT_FIELD

#define INT_FIELD(x, help) \
NMI_GET( CharacterWrapper, x, help) \
{ \
    CHK_PC \
    checkTarget( ); \
    return Register( (int) target->getNPC()->x ); \
} \
NMI_SET( CharacterWrapper, x, help) \
{ \
    CHK_PC \
    checkTarget( ); \
    target->getNPC()->x = arg.toNumber(); \
}
INT_FIELD(off_flags, "флаги поведения моба (таблица .tables.off_flags)")

NMI_GET(CharacterWrapper, last_fight_delay, "задержка после боя в секундах")
{
    checkTarget();
    return (int)target->getLastFightDelay();
}

NMI_GET(CharacterWrapper, adrenaline, "полна ли кровь адреналина")
{
    checkTarget();
    return target->is_adrenalined();
}

NMI_GET(CharacterWrapper, afterCharm, "очарован или недавно раз-очарован")
{
    checkTarget();
    if (target->is_npc())
        return target->getNPC()->behavior && target->getNPC()->behavior->isAfterCharm();
    else
        return IS_CHARMED(target);
}

NMI_GET(CharacterWrapper, start_room, "в какой комнате зашли в мир")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getStartRoom();
}

NMI_SET(CharacterWrapper, start_room, "в какой комнате зашли в мир")
{
    checkTarget();
    CHK_NPC
    target->getPC()->setStartRoom(arg2number(arg));
}

NMI_GET(CharacterWrapper, loyalty, "лояльность по отношению к закону")
{
    checkTarget();
    CHK_NPC
    return target->getPC()->getLoyalty();
}

NMI_SET(CharacterWrapper, loyalty, "лояльность по отношению к закону")
{
    checkTarget();
    CHK_NPC
    target->getPC()->setLoyalty(arg2number(arg));
}


NMI_SET( CharacterWrapper, wearloc, "названия всех слотов экипировки через пробел")
{
    checkTarget( );
    target->wearloc.fromString( arg.toString( ) );
}

NMI_GET( CharacterWrapper, wearloc, "названия всех слотов экипировки через пробел")
{
    checkTarget( );
    return target->wearloc.toString();
}

NMI_GET( CharacterWrapper, max_carry_weight, "макс вес, который может нести персонаж, 0 для петов, 100500 для богов")
{
    checkTarget( );
    return target->canCarryWeight();
}

NMI_GET( CharacterWrapper, max_carry_number, "макс кол-во вещей, которое может нести персонаж, 0 для петов, 1000 для богов")
{
    checkTarget( );
    return target->canCarryNumber();
}

NMI_GET(CharacterWrapper, carry_weight, "вес, который несет персонаж")
{
    checkTarget();
    return target->getCarryWeight();
}


NMI_GET( CharacterWrapper, expToLevel, "сколько опыта осталось набрать до след уровня")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getExpToLevel( );
}

NMI_GET( CharacterWrapper, hostname, "IP-адрес соединения")
{
    checkTarget( );

    if (!target->desc)
        return "";
    else
        return target->desc->getRealHost( );
}

NMI_GET( CharacterWrapper, modifyLevel, "уровень с учетом бонусов от ремортов" )
{
    checkTarget( );
    return target->getModifyLevel( );
}

NMI_GET( CharacterWrapper, level, "настоящий уровень" )
{
    checkTarget( );
    return target->getRealLevel( );
}

NMI_SET( CharacterWrapper, level, "настоящий уровень" )
{
    checkTarget( );
    return target->setLevel( arg.toNumber( ) );
}

NMI_GET( CharacterWrapper, newbie, "true если нет ремортов, <50 квестов и самооценка новичок")
{
    checkTarget();
    CHK_NPC
    return is_total_newbie(target->getPC());
}

NMI_GET( CharacterWrapper, lastAccessTime, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getLastAccessTime( ).getTimeAsString( );
}

NMI_GET( CharacterWrapper, profession, "класс (структура .Profession)" )
{
    checkTarget( );
    return Register::handler<ProfessionWrapper>(target->getProfession()->getName());
}

NMI_SET( CharacterWrapper, profession, "класс (структура .Profession)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setProfession( "none" );
    else
        target->getPC( )->setProfession( wrapper_cast<ProfessionWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, religion, "религия (структура .Religion)" )
{
    checkTarget( );
    CHK_NPC
    return Register::handler<ReligionWrapper>( target->getPC( )->getReligion( )->getName( ) );
}

NMI_GET( CharacterWrapper, godName, "название религии, случайный бог для неопределившихся или строка 'бог|и|ов...' для мобов" )
{
    checkTarget( );
    return ReligionUtils::godName(target);    
}

NMI_SET( CharacterWrapper, religion, "религия (структура .Religion)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setReligion( "none" );
    else
        target->getPC( )->setReligion( wrapper_cast<ReligionWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, hometown, "родной город (структура .Hometown)" )
{
    checkTarget( );
    CHK_NPC
    return HometownWrapper::wrap( target->getPC( )->getHometown( )->getName( ) );
}

NMI_SET( CharacterWrapper, hometown, "родной город (структура .Hometown)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setHometown( "none" );
    else
        target->getPC( )->setHometown( wrapper_cast<HometownWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, clan, "клан (структура .Clan)" )
{
    checkTarget( );
    return ClanWrapper::wrap( target->getClan( )->getName( ) );
}

NMI_SET(CharacterWrapper, on, "объект, мебель, на которой сидим")
{
    checkTarget( );
    ::Object *obj = arg2item(arg);
    target->on = obj;
}

NMI_SET( CharacterWrapper, russianName, "русские имена с падежами" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->setRussianName( arg.toString( ) );
}

NMI_GET( CharacterWrapper, russianName, "русские имена с падежами" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getRussianName( ).getFullForm( );
}

NMI_SET( CharacterWrapper, name, "имя" )
{
    checkTarget( );
    target->setName( arg.toString( ) );
}

NMI_GET( CharacterWrapper, name, "имя" )
{
    checkTarget( );
    return target->getName( );
}
NMI_GET( CharacterWrapper, names, "имя персонажа или короткое описание моба с падежами" )
{
    checkTarget( );
    return target->toNoun()->getFullForm();
}

NMI_GET( CharacterWrapper, race, "раса (структура .Race)" )
{
    checkTarget( );
    return RaceWrapper::wrap( target->getRace( )->getName( ) );
}

NMI_SET( CharacterWrapper, race, "раса (структура .Race)" )
{
    checkTarget( );
    if (arg.type == Register::NONE)
        target->setRace( "none" );
    else
        target->setRace( wrapper_cast<RaceWrapper>(arg)->name );
}

NMI_GET( CharacterWrapper, charmed, "true если очарован и есть хозяин" )
{
    checkTarget();
    return IS_CHARMED(target);
}

NMI_GET( CharacterWrapper, vampire, "true если персонаж в форме вампира или моб-вампир" )
{
    checkTarget();
    if (!target->is_npc())
        return target->is_vampire();
    else
        return IS_MOB_VAMPIRE(target);
}

NMI_GET(CharacterWrapper, followers, "список существ под очарованием, у которых персонаж master" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (Character *wch = char_list; wch; wch = wch->next) {
        if (IS_CHARMED(wch) && wch->master == target)
            rc->push_back(wrap(wch));
    }

    return wrap(rc);
}

NMI_GET( CharacterWrapper, connected, "true если есть связь" )
{
    Character *ch;
    
    checkTarget( );
    
    if (!target->is_npc( ) && target->getPC( )->switchedTo)
        ch = target->getPC( )->switchedTo;
    else
        ch = target;

    return (ch->desc != NULL);
}

NMI_GET( CharacterWrapper, isInInterpret, "true если игрок в состоянии ввода команд (не ed, не olc, не pager, etc)" )
{
    checkTarget();
    CHK_NPC
    return Register(target->desc && target->desc->handle_input.front( )->getType() == "InterpretHandler");
}

/*
 * METHODS
 */

NMI_INVOKE( CharacterWrapper, ptc, "(msg): print to char, печатает строку msg" )
{
    checkTarget( );
    DLString d = args.front().toString();
    page_to_char(d.c_str(), target);
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret, "(msg): интерпретирует строку msg, как будто чар ее набрал сам" )
{
    checkTarget( );

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    DLString d = args.front().toString();
    return ::interpret( target, d.c_str() );
}

NMI_INVOKE( CharacterWrapper, interpret_raw, "(cmd, arg): выполняет команду с аргументами от имени чара, без предварительных проверок" )
{
    DLString cmdName, cmdArgs;
    RegisterList::const_iterator i;
    checkTarget( );

    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    cmdName = i->toString( );

    if (++i != args.end( ))
        cmdArgs = i->toString( );

    ::interpret_raw( target, cmdName.c_str( ), cmdArgs.c_str( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret_cmd, "(cmd, args): выполняет команду с аргументами от имени чара" )
{
    DLString cmdName, cmdArgs;
    RegisterList::const_iterator i;
    checkTarget( );

    if (args.size( ) < 1)
        throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    cmdName = i->toString( );

    if (++i != args.end( ))
        cmdArgs = i->toString( );

    ::interpret_cmd( target, cmdName.c_str( ), cmdArgs.c_str( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, get_char_world, "(name[,flags]): найти персонажа в мире по имени name, с флагоми поиска (таблица .tables.find_flags)" )
{
    checkTarget( );
    DLString name = argnum2string(args, 1);
    bitstring_t flags = args.size() > 1 ? argnum2flag(args, 2, find_flags) : 0;

    return wrap(::get_char_world(target, name, flags));
}

NMI_INVOKE( CharacterWrapper, get_obj_here, "(name): видимый нам объект в комнате, инвентаре или equipment" )
{
    checkTarget( );
    return wrap( ::get_obj_here( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_carry_type, "(type): видимый нам объект в инвентаре или equipment с этим типом (таблица .tables.item_table)" )
{
    checkTarget( );
    return wrap( ::get_obj_carry_type( target, args2number( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_liquid_carry, "(liqname): вернет емкость в инвентаре с заданной жидкостью" )
{
    checkTarget( );

    DLString liqName = args2string(args);
    Liquid *liquid = liquidManager->find(liqName);
    if (!liquid)
        throw Scripting::Exception( "Invalid liquid name");

    list< ::Object *> drinks = ::get_objs_list_type(target, ITEM_DRINK_CON, target->carrying);
    for (list< ::Object *>::iterator o = drinks.begin(); o != drinks.end(); o++)
        if ((*o)->wear_loc == wear_none)
            if (liquidManager->find((*o)->value2()) == liquid)
                return wrap(*o);

    return Register();
}

NMI_INVOKE( CharacterWrapper, get_recipe_carry, "(flag): вернет рецепт в инвентаре с заданным флагом (таблица .tables.recipe_flags)" )
{
    checkTarget( );

    bitstring_t flag = args2number(args);
    list< ::Object *> recipes = ::get_objs_list_type(target, ITEM_RECIPE, target->carrying);
    for (list< ::Object *>::iterator o = recipes.begin(); o != recipes.end(); o++)
        if ((*o)->wear_loc == wear_none)
            if (IS_SET((*o)->value0(), flag))
                return wrap(*o);

    return Register();
}
NMI_INVOKE( CharacterWrapper, get_obj_room, "(name): поиск по имени видимого объекта в комнате" )
{
    checkTarget( );
    return wrap( ::get_obj_room( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_wear, "(name): поиск по имени видимого объекта в экипировке" )
{
    checkTarget( );
    return wrap( ::get_obj_wear( target, args2string( args ) ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_wear_vnum, "(vnum): поиск объекта в экипировке по внуму" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum && obj->wear_loc != wear_none)
            return wrap( obj );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, get_char_room, "(name[,room]): поиск по имени видимого персонажа, в той же комнате или в room" )
{
    checkTarget( );
    
    Room *room;
    DLString name = args2string( args );

    if (args.size( ) == 2)
        room = arg2room( args.back( ) );
    else
        room = target->in_room;
    
    return wrap( ::get_char_room( target, room, name ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_inventory, "(name): поиск объекта в инвентаре, по имени или ID" )
{
    checkTarget( );

    DLString objname = argnum2string(args, 1);
    
    return wrap( ::get_obj_carry( target, objname ) );
}

NMI_INVOKE( CharacterWrapper, get_obj_carry, "(name[,looker]): поиск объекта в экипировке или видимого (себе или персонажу looker) объекта в инвентаре, по имени или ID" )
{
    checkTarget( );

    DLString objname = argnum2string(args, 1);
    Character *looker = args.size() <= 1 ? 0 : argnum2character(args, 2);
    
    return wrap( ::get_obj_wear_carry( target, objname, looker ) );
}


NMI_INVOKE( CharacterWrapper, transfer, "(room,actor,msgRoomLeave,msgSelfLeave,msgRoomEnter,msgSelfEnter): actor переносит нас в комнату room" )
{
    Room *room;
    Character *actor;
    RegisterList::const_iterator i = args.begin( );
    DLString m1, m2, m3, m4;
    
    checkTarget( );

    if (args.size( ) != 6)
        throw Scripting::NotEnoughArgumentsException( );
    
    room = arg2room( *i );
    actor = arg2character( *++i );
    m1 = (++i)->toString();
    m2 = (++i)->toString();
    m3 = (++i)->toString();
    m4 = (++i)->toString();
    transfer_char( target, actor, room, m1.c_str(), m2.c_str(), m3.c_str(), m4.c_str() );
    
    return Register( );
}

NMI_INVOKE( CharacterWrapper, char_to_room, "(room): поместить в комнату room")
{
    checkTarget( );
    Room *room = arg2room( get_unique_arg( args ) ); 
    
    if (target->in_room) {
        undig( target );
        target->dismount( );
        ::char_from_room( target );
    }

    ::char_to_room( target, room );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, is_npc, "(): true для мобов, false для игроков" )
{
    checkTarget( );
    return Register( (int)target->is_npc( ) );
}

NMI_INVOKE( CharacterWrapper, getName, "(): имя игрока или список имен моба" )
{
    checkTarget( );
    return Register( target->getName() );
}

NMI_INVOKE( CharacterWrapper, setName, "(name): устанавливает имена моба" )
{
    checkTarget( );
    CHK_PC
    target->setName( args2string( args ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, seeName, "(ch[, case]): как мы видим имя и претитул ch в падеже case") 
{
    checkTarget( );
    int cse = 1;
    
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );

    Character *ch = arg2character( *i );

    i++;
    
    if(i != args.end())
        cse = i->toNumber();
        
    return Register( target->seeName(ch, '0' + cse ) );
}

NMI_INVOKE( CharacterWrapper, getParsedTitle, "(): титул персонажа как мы его видим" )
{
    checkTarget();
    CHK_NPC
    return Register( target->getPC()->getParsedTitle() );
}

NMI_INVOKE( CharacterWrapper, can_see_mob, "(ch): видим ли персонажа ch" )
{
    checkTarget( );
    return target->can_see( arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_obj, "(obj): видим ли предмет obj" )
{
    checkTarget( );
    return target->can_see( arg2item( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_room, "(room): видим ли комнату room" )
{
    checkTarget( );
    return target->can_see( arg2room( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, can_see_exit, "(door): видим ли выход под номером door" )
{
    int door;
    EXIT_DATA *pExit;

    checkTarget( );
    door = args2number( args );
    if (door < 0 || door >= DIR_SOMEWHERE)
        throw Scripting::IllegalArgumentException( );

    if (!( pExit = target->in_room->exit[door] ))
        return false;

    return target->can_see( pExit );
}

NMI_INVOKE( CharacterWrapper, print, "(fmt, args): возвращает отформатированную строку (см. статью вики про ф-ии вывода)" )
{
    checkTarget();
    
    return Register( regfmt(target, args) );
}

NMI_INVOKE( CharacterWrapper, act, "(fmt, args): печатает нам отформатированную строку (с символом конца строки). " )
{
    checkTarget();
    
    target->pecho( regfmt(target, args) );
    
    return Register( );
}

NMI_INVOKE( CharacterWrapper, echoMaster, "(fmt, args): выдать строку хозяину, если он есть и отдал этот приказ" )
{
    checkTarget();

    bool needsOutput = IS_CHARMED(target) 
            && target->master->getPC() 
            && target->master->getPC()->getAttributes().isAvailable("ordering");

    if (needsOutput) {
        DLString msg = fmt(0, "{W%#^C1 {Wне может выполнить твой приказ, потому что видит следующее:{x\r\n  {W*{x ", target);
        target->master->pecho( msg + regfmt(target->master, args) );
        return true;
    }
    
    return false;
}

NMI_INVOKE( CharacterWrapper, recho, "(fmt, args): выводит отформатированную строку всем в комнате, кроме нас" )
{
    checkTarget();
    if (!target->in_room)
        return Register();

    for (Character *to = target->in_room->people; to; to = to->next_in_room) {
        if (to == target)
            continue;
        if (!to->can_sense(target))
            continue;

        to->pecho(POS_RESTING, regfmt(to, args).c_str());
    }

    return Register();
}

NMI_INVOKE( CharacterWrapper, rvecho, "(vict, fmt, args...): выводит отформатированную строку всем в комнате, кроме нас и vict" )
{
    checkTarget();
    if (!target->in_room)
        return Register();

    RegisterList myArgs(args);
    Character *vict = args2character(args);
    myArgs.pop_front();

    for (Character *to = target->in_room->people; to; to = to->next_in_room) {
        if (to == target || to == vict)
            continue;            
        if (!to->can_sense(target))
            continue;

        to->pecho(POS_RESTING, regfmt(to, myArgs).c_str());
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, say, "(format, args...): произносит вслух реплику, отформатированную как в методе act" )
{
    checkTarget( );

    for (Character *to = target->in_room->people; to; to = to->next_in_room) {
        if (to == target)
            continue;
        if (!to->can_sense(target))
            continue;

        DLString msg = regfmt(to, args);
        to->pecho(POS_RESTING, "%^C1 произносит '{g%s{x'", target, msg.c_str());
    }

    return Register();
}

NMI_INVOKE( CharacterWrapper, psay, "(ch, format, args...): произносит вслух реплику, отформатированную как в методе act и видимую только для ch" )
{
    checkTarget( );
    RegisterList myArgs(args);
    Character *ch= args2character(args);
    myArgs.pop_front();

    DLString msg = regfmt(ch, myArgs);
    ch->pecho("%^C1 произносит '{g%s{x'", target, msg.c_str());
    return Register();
}


NMI_INVOKE( CharacterWrapper, getModifyLevel, "(): уровень, с учетом плюшек от ремортов" )
{
    checkTarget();
    
    return target->getModifyLevel();
}

NMI_INVOKE( CharacterWrapper, getRealLevel, "(): настоящий уровень" )
{
    checkTarget();
    
    return target->getRealLevel();
}

NMI_INVOKE( CharacterWrapper, getSex, "(): номер пола (0 neutral, 1 male, 2 female, 3 random - только у прототипов)" )
{
    checkTarget();
    
    return target->getSex();
}

NMI_INVOKE( CharacterWrapper, is_immortal, "(): true, если this бессмертный или кодер" )
{
    checkTarget();
    
    return target->is_immortal();
}

NMI_INVOKE( CharacterWrapper, edit, "(): переводит this в режим редактирования" )
{
    checkTarget();
    
    PCharacter *pch = target->getPC();
    
    if(!pch)
        throw Scripting::Exception( "only for PCs" );
    
    DLString str;
    
    XMLEditorInputHandler::Pointer eih( NEW );
    
    if(!args.empty()) {
        eih->clear( );
        eih->setBuffer(args.front().toString());
    }

    eih->attach(pch);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, edReg, "([ndx[, txt]]): возвращает/устанавливает содержимое регистров редактора" )
{
    RegisterList::const_iterator i = args.begin( );

    checkTarget();
    
    PCharacter *pch = target->getPC();
    
    if(!pch)
        throw Scripting::Exception( "only for PCs" );
    
    unsigned char ndx = 0;

    if(i != args.end()) {
        ndx = i->toNumber();
        i++;
    }

    Editor::reg_t &reg = pch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")->regs[ndx];

    DLString str;

    if(i == args.end())
        for(Editor::reg_t::const_iterator j = reg.begin(); j != reg.end(); j++)
            str.append(*j).append("\n");
    else 
        reg.split(str = i->toString());

    return Register(str);
}


NMI_INVOKE( CharacterWrapper, gainExp, "(exp): добавляет exp очков опыта" )
{
    checkTarget( );
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );
    
    CHK_NPC
    target->getPC()->gainExp(i->toNumber());

    return Register();
}

NMI_INVOKE( CharacterWrapper, getClass, "(): строка с названием класса" )
{
    checkTarget();
    return Register( target->getProfession( )->getName( ).c_str( ) );
}
NMI_INVOKE( CharacterWrapper, getClan, "(): строка с названием клана" )
{
    checkTarget();
    return Register( target->getClan( )->getShortName( ) );
}
NMI_INVOKE( CharacterWrapper, setClan, "(name): устанавливает клан по строке с именем" )
{
    Clan *clan;
    
    checkTarget();

    if (args.empty())
        throw Scripting::NotEnoughArgumentsException( );
    
    clan = ClanManager::getThis( )->findUnstrict( args.front( ).toString( ) );

    if (!clan)
        throw Scripting::IllegalArgumentException( );
    else
        target->setClan( clan->getName( ) );
    
    return Register( );
}
NMI_INVOKE( CharacterWrapper, getClanLevel, "(): клановый уровень, число от 0 до 8" )
{
    checkTarget();
    CHK_NPC
    return Register( target->getPC()->getClanLevel() );
}
NMI_INVOKE( CharacterWrapper, setClanLevel, "(уровень): клановый уровень, число от 0 до 8" )
{
    checkTarget();
    CHK_NPC
    target->getPC()->setClanLevel(args2number(args));
    return Register();
}
NMI_INVOKE( CharacterWrapper, getRace, "(): строка с названием расы" )
{
    checkTarget();
    return Register( target->getRace( )->getName( ) );
}

NMI_INVOKE( CharacterWrapper, extract, "(bool): уничтожить полностью (suicide/remort игрока или смерть моба) или не полностью как при выходе из мира" )
{
    checkTarget( );
    RegisterList::const_iterator i = args.begin( );

    if(i == args.end())
        throw Scripting::NotEnoughArgumentsException( );
    
    extract_char(target, i->toNumber());
    return Register();
}

NMI_INVOKE( CharacterWrapper, add_follower, "(master): делает нас последователем master-а" )
{
    checkTarget( );
    follower_add(target, arg2character( get_unique_arg( args ) ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, stop_follower, "([verbose]): прекращает следование, снимает очарование" )
{
    checkTarget( );

    bool verbose = true;
    if (args.size() > 0)
        verbose = args.front().toBoolean();

    follower_stop(target, verbose);
    return Register();
}

NMI_INVOKE( CharacterWrapper, is_same_group, "(gch): вернет true если мы с персонажем gch в одной группе" )
{
    checkTarget( );
    Character *gch = arg2character(get_unique_arg(args));
    return Register(is_same_group(target, gch));
}

NMI_GET(CharacterWrapper, groupHere, "список (List) всех согруппников в комнате")
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (Character *rch = target->in_room->people; rch; rch = rch->next_in_room)
        if (is_same_group(target, rch))
            rc->push_back( WrapperManager::getThis( )->getWrapper( rch ) );
    
    return wrap(rc);
}

NMI_INVOKE( CharacterWrapper, clearBehavior, "(): сбросить поведение моба до обычного" )
{
    checkTarget( );
    CHK_PC
    MobileBehaviorManager::assignBasic( target->getNPC( ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, rememberFought, "(ch): запомнить персонажа ch как будто с ним сражались" )
{
    checkTarget();
    CHK_PC

    if (!target->getNPC()->behavior)
        return Register(false);

    BasicMobileBehavior::Pointer ai = target->getNPC()->behavior.getDynamicPointer<BasicMobileBehavior>();
    if (!ai)
        return Register(false);

    Character *ch = args2character(args);
    ai->rememberFought(ch);
    return Register(true);
}

NMI_INVOKE( CharacterWrapper, get_random_room, "(): случайная комната, куда можно зайти" )
{
    checkTarget( );
    
    RoomVector rooms;
    
    for (auto &r: roomInstances)
        if (target->canEnter(r) && !r->isPrivate())
            rooms.push_back(r);
    
    if (rooms.empty())
        return Register( );
    else {
        Room *r = rooms[::number_range(0, rooms.size() - 1)];
        return WrapperManager::getThis( )->getWrapper(r); 
    }
}

NMI_INVOKE( CharacterWrapper, is_safe, "(vict): защищают ли боги vict от нас" )
{
    checkTarget( );
    return ::is_safe_nomessage( target, 
                                arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, is_safe_spell, "(vict): защищают ли боги vict от наших арийных заклинаний" )
{
    checkTarget();
    return ::is_safe_spell(target,
                           args2character(args),
                           true);
}

NMI_INVOKE( CharacterWrapper, is_safe_rspell, "(af): защищают ли боги от действия заклинания аf на комнате" )
{
    checkTarget();
    Affect *paf = args2affect(args);
    return ::is_safe_rspell(paf, target, true);
}

NMI_INVOKE( CharacterWrapper, rawdamage, "(vict,dam,damtype[,label]): нанести vict повреждения в размере dam с типом damtype (таблица .tables.damage_table)" )
{
    RegisterList::const_iterator i;
    Character *victim;
    int dam;
    int dam_type = DAM_NONE;
    DLString label;

    checkTarget( );

    if (args.size() < 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    victim = argnum2character(args, 1);
    dam = argnum2number(args, 2);
    dam_type = argnum2flag(args, 3, damage_table);
    if (args.size() > 3)
        label = argnum2string(args, 4);

    ::rawdamage_nocatch(target, victim, dam_type, dam, true, label);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, setViolent, "(vict): установить VIOLENT за нападение на vict" )
{
    checkTarget();
    CHK_NPC
    Character *victim = args2character(args);
    ::set_violent( target, victim, true );
    return Register();
}

NMI_INVOKE( CharacterWrapper, damage, "(vict,dam,skillName,damtype,damflags[,show]): нанести vict повреждения в размере dam умением skillName и типом damtype (таблица .tables.damage_table)" )
{
    checkTarget( );
    Character *victim = argnum2character(args, 1);
    int dam = argnum2number(args, 2);
    Skill *skill = argnum2skill(args, 3);
    int dam_type = argnum2flag(args, 4, damage_table);
    bitstring_t damflags = argnum2flag(args, 5, damage_flags);
    bool show = args.size() > 5 ? argnum2boolean(args, 6) : true;

    return ::damage_nocatch(target, victim, dam, skill->getIndex( ), dam_type, show, damflags);
}

NMI_INVOKE( CharacterWrapper, one_hit, "(vict): нанести vict один удар оружием" )
{
    checkTarget();
    Character *victim = args2character(args);
    ::one_hit(target, victim);
    return Register();
}

NMI_INVOKE( CharacterWrapper, saves_spell, "(caster,level,dam_type[,dam_flag[,verbose]]): спас-бросок против типа повреждения (.tables.damage_table) с флагом повреждения (.tables.damage_flags)")
{
    checkTarget();
    Character *caster = argnum2character(args, 1);
    int level = argnum2number(args, 2);
    int dam_type = argnum2flag(args, 3, damage_table);
    int dam_flag = DAMF_OTHER;
    if (args.size() > 3)
        dam_flag = argnum2flag(args, 4, damage_flags);

    bool verbose = true;
    if (args.size() > 4)
        verbose = argnum2boolean(args, 5);
        

    return Register(saves_spell(level, target, dam_type, caster, dam_flag, verbose));
}

NMI_INVOKE(CharacterWrapper, quaff, "(obj): получить эффекты от пилюли или зелья")
{
    checkTarget();
    ::Object *item = argnum2item(args, 1);

    if (item->item_type != ITEM_POTION && item->item_type != ITEM_PILL)
        throw Scripting::Exception("Object is not a pill or a potion");

    spell_by_item(target, item);
    return Register();
}

NMI_INVOKE( CharacterWrapper, spell, "(skillName,level[,vict|argument[,spellbane[,verbose]]]): скастовать заклинания на всю комнату, на vict или с аргументом")
{
    checkTarget( );

    Skill *skill = argnum2skill(args, 1);
    int level = argnum2number(args, 2);
    if (args.size() == 2) {
        // Room spell.
        spell( skill->getIndex( ), level, target, target->in_room );
        return Register();
    }

    Register arg3 = argnum(args, 3);
    if (arg3.type == Register::STRING) {
        // String argument spell.
        const char *arg = arg3.toString().c_str();
        spell( skill->getIndex( ), level, target, const_cast<char *>(arg) );
        return Register();
    } 

    // Character target spell.    
    Character *victim = argnum2character(args, 3);
    if (!victim)
        throw Scripting::IllegalArgumentException( );

    // Figure out the flags.
    int flags = 0;
    if (args.size() >= 4 && argnum2number(args, 4))
        SET_BIT(flags, FSPELL_BANE);
    if (args.size() >= 5 && argnum2number(args, 5))
        SET_BIT(flags, FSPELL_VERBOSE);
 
    ::spell( skill->getIndex( ), level, target, victim, flags );
    return Register();
}

NMI_INVOKE( CharacterWrapper, multi_hit, "(vict): нанести один раунд повреждений жертве" )
{
    checkTarget( );
    ::multi_hit( target, arg2character( get_unique_arg( args ) ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, raw_kill, "([flags[,killer[,label[,damtype]]]]): убить. флаги из таблицы .tables.death_flags" )
{
    RegisterList::const_iterator i;
    Character *killer = NULL;
    DLString label;
    int damtype = -1; 
    bitstring_t flags = 0;

    checkTarget();
    
    if (args.size() > 0)
        flags = argnum2flag(args, 1, death_flags);
    flags = std::max(0LL, flags);
    if (args.size() > 1)
        killer = argnum2character(args, 2);
    if (args.size() > 2)
        label = argnum2string(args, 3);
    if (args.size() > 3)
        damtype = argnum2flag(args, 4, damage_table);
    
    raw_kill( target, flags, killer, label, damtype );
    throw VictimDeathException();
}

NMI_INVOKE( CharacterWrapper, affectAdd, "(.Affect): повесить новый аффект" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    affect_to_char( target, &(aw->getTarget()) );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectJoin, "(.Affect): повесить новый аффект или усилить существующий" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    affect_join( target, &(aw->getTarget()) );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectBitStrip, "(where,bit): снять все аффекты, устанавливающие в поле where (.tables.affwhere_flags) значение bit")
{
    int bits;
    
    checkTarget( );

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    // FIXME: change affectBitStrip in existing codesources.
    bits = args.back( ).toNumber( );
    affect_bit_strip( target, &affect_flags, bits );
    return Register( ); 
}

NMI_INVOKE( CharacterWrapper, isAffected, "(skillName): находится ли под воздействием аффекта с именем skillName" )
{
    Skill *skill;
    
    checkTarget( );

    if (args.size( ) != 1)
        throw Scripting::NotEnoughArgumentsException( );

    skill = skillManager->findExisting( args.front( ).toString( ) );

    if (skill)
        return target->isAffected( skill->getIndex( ) );
    else
        return false;
}

NMI_INVOKE( CharacterWrapper, affectStrip, "(skillName[,verbose]): снять все аффекты с именем skillName, показав сообщение о спадании (verbose)" )
{
    checkTarget( );
    Skill *skill = argnum2skill(args, 1);
    bool verbose = args.size() > 1 ? argnum2boolean(args, 2) : false;
        
    affect_strip( target, skill->getIndex( ), verbose );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectReplace, "(.Affect): удалить все аффекты этого типа и повесить новый" )
{
    checkTarget( );
    AffectWrapper *aw;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );        
    affect_strip(target, aw->getTarget().type);
    affect_to_char( target, &(aw->getTarget()) );
    return Register( );
}


NMI_INVOKE( CharacterWrapper, affectRemoveAll, "(): снять все аффекты" )
{
    checkTarget();

    for (auto &paf: target->affected.clone())
        affect_remove( target, paf );

    return Register();
}

NMI_INVOKE( CharacterWrapper, isVulnerable, "(damtype, damflag): есть ли уязвимость к типу повреждений из .tables.damage_table с флагом повреждений из .tables.damage_flags" )
{
    checkTarget();
    int damtype = argnum2flag(args, 1, damage_table);
    int damflag = argnum2flag(args, 2, damage_flags);
    return immune_check(target, damtype, damflag) == RESIST_VULNERABLE;
}

NMI_INVOKE( CharacterWrapper, isImmune, "(damtype, damflag): есть ли иммунитет к типу повреждений из .tables.damage_table с флагом повреждений из .tables.damage_flags" )
{
    checkTarget();
    int damtype = argnum2flag(args, 1, damage_table);
    int damflag = argnum2flag(args, 2, damage_flags);
    return immune_check(target, damtype, damflag) == RESIST_IMMUNE;
}

NMI_INVOKE( CharacterWrapper, stop_fighting, "(): прекратить битву" )
{
    checkTarget( );
    stop_fighting(target, get_unique_arg( args ).toBoolean( ));
    return Register( );
}

NMI_INVOKE( CharacterWrapper, move_char, "(door[,movetype]): переместить персонажа в дверь door, с типом движения movetype('running','crawl'). Вернет true если переместили.")
{
    int door, rc;
    DLString movetypeName;
    
    checkTarget( );

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );
    
    if (args.size( ) > 2)
        throw Scripting::TooManyArgumentsException( );
    
    door = args.front( ).toNumber( );
    if (door < 0 || door >= DIR_SOMEWHERE)
        return false;
    
    if (args.size( ) > 1)
        movetypeName = args.back( ).toString( );
    else 
        movetypeName = "normal";

    rc = ::move_char( target, door, movetypeName.c_str( ) );
    return Register( rc == RC_MOVE_OK );
}

NMI_INVOKE( CharacterWrapper, dismount, "(): спешиться или сбросить всадника")
{
    checkTarget();
    target->dismount();
    return Register();
}

NMI_INVOKE( CharacterWrapper, addDarkShroud, "(): повесить темную ауру")
{
    Affect af;
    
    checkTarget( );

    af.type      = gsn_dark_shroud;
    af.level     = target->getRealLevel( );
    af.duration  = -1;
    affect_to_char( target, &af );

    return Register( );
}    

NMI_INVOKE( CharacterWrapper, isLawProtected, "(): охраняется ли моб законом" )
{
    NPCharacter *mob;
    
    checkTarget();
    CHK_PC
    mob = target->getNPC( );

    if (IS_SET(mob->pIndexData->area->area_flag, AREA_HOMETOWN))
        return true;

    return false;
}

NMI_INVOKE( CharacterWrapper, can_get_obj, "(obj): может ли поднять предмет obj с земли" )
{
    checkTarget( );

    ::Object *obj = arg2item( get_unique_arg( args ) );

    if (!obj->can_wear( ITEM_TAKE )) 
        return false;
    if (!obj->getOwner().empty())
        return false;
//     if (obj->behavior)
//         return false;
    if (!target->can_see( obj ))
        return false;
    if (obj->isAntiAligned( target ))
        return false;

    return true;
}

NMI_GET(CharacterWrapper, totems, "список всех тотемов, созданных персонажем" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (::Object *obj = object_list; obj != 0; obj = obj->next) {
        if (obj->item_type != ITEM_FURNITURE)
            continue;
        if (!IS_SET(obj->value2(), TOTEM))
            continue;
        if (!obj->hasOwner(target))
            continue;
        
        rc->push_back(WrapperManager::getThis( )->getWrapper(obj));
    }

    return wrap(rc);
}

NMI_INVOKE(CharacterWrapper, list_obj_world, "(arg): поиск по миру видимых персонажу предметов с уровнем не выше персонажа" )
{
    checkTarget();
    DLString arg = args2string(args);
    RegList::Pointer rc(NEW);

    for (::Object *obj = object_list; obj != 0; obj = obj->next) {
        if (target->getRealLevel() < get_wear_level(target, obj))
            continue;

        if (!target->can_see(obj))
            continue;

        Character *carrier = obj->getCarrier();
        if (carrier && !target->can_see(carrier))
            continue;

        Room *location = obj->getRoom();
        if (location && !target->can_see(location))
            continue;

        if (!obj_has_name(obj, arg, target))
            continue;

        rc->push_back(WrapperManager::getThis( )->getWrapper(obj));
    }
    
    return wrap(rc);
}


NMI_INVOKE(CharacterWrapper, get_obj_carry_vnum, "(vnum): поиск по внуму предмета в инвентаре или экипировке" )
{
    checkTarget( );

    int vnum = args2number( args );

    for (::Object *obj = target->carrying; obj; obj = obj->next_content)
        if (obj->pIndexData->vnum == vnum)
            return wrap( obj );

    return Register( );
}

NMI_INVOKE(CharacterWrapper, can_drop_obj, "(obj): может ли избавиться от предмета obj в инвентаре" )
{
    checkTarget( );
    ::Object *obj = arg2item( get_unique_arg( args ) );
    return ::can_drop_obj(target, obj, false);
}

NMI_INVOKE( CharacterWrapper, mortality, "(): включает-выключает бессмертие для кодеров" )
{
    checkTarget( );

    if (target->is_npc( ) || !target->getPC( )->getAttributes( ).isAvailable( "mortality" ))
        throw Scripting::Exception( "Attribute not found" );
    
    if (target->getPC( )->getAttributes( ).isAvailable( "coder" )) {
        target->getPC( )->getAttributes( ).eraseAttribute( "coder" );
        target->getPC( )->setSecurity( 0 );
        target->pecho("Now you are mortal.");
        return 1;
    }
    else {
        target->getPC( )->getAttributes( ).getAttr<XMLAttributeCoder>( "coder" );
        target->getPC( )->setSecurity( 999 );
        target->pecho("Now you are immortal.");
        return 0;
    }
}

NMI_INVOKE( CharacterWrapper, echoOn, "(): включает отображение введенного текста в терминале" )
{
    checkTarget( );
    
    if (target->desc)
        target->desc->echoOn( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, echoOff, "(): выключает отображение введенного текста в терминале" )
{
    checkTarget( );
    
    if (target->desc)
        target->desc->echoOff( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, save, "(): сохранить профайл на диск" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->save( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, skills, "([origin]): список названий доступных скилов, всех или с данным происхождением (.tables.skill_origin_table)" )
{
    checkTarget();
    CHK_NPC

    RegList::Pointer list(NEW);
    int origin = args.empty() ? NO_FLAG : argnum2flag(args, 1, skill_origin_table);
    
    for (int sn = 0; sn < skillManager->size(); sn++) {
        Skill *skill = skillManager->find(sn);
        // Choose only permanent skills available at any level, or active temporary skills.
        if (!skill->visible(target))
            continue;

        PCSkillData &data = target->getPC()->getSkillData(sn);
        // Filter by requested origin (fenia, religion, etc) or return all.
        if (origin != NO_FLAG && data.origin != origin)
            continue;

        list->push_back(Register(skill->getName()));
    }

    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

NMI_INVOKE( CharacterWrapper, updateSkills, "(): освежить разученность умений (при входе в мир)" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->updateSkills( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, hasAttribute, "(attr): true если установлен аттрибут с именем attr" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getAttributes( ).isAvailable( args2string( args ) );
}

NMI_INVOKE( CharacterWrapper, eraseAttribute, "(attr): удаляет аттрибут с именем attr" )
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->getAttributes( ).eraseAttribute( args2string( args ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, visible, "(): проявиться из невидимости" )
{
    checkTarget();
    do_visible(target);
    return Register();
}

NMI_INVOKE( CharacterWrapper, get_eq_char, "(wearloc): предмет экипировки, надетый на эту локацию" )
{
    checkTarget( );
    return wrap( arg2wearloc( get_unique_arg( args ) )->find( target ) );
}

NMI_INVOKE( CharacterWrapper, hasWearloc, "(wearloc): обладает ли данным слотом в экипировке")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getWearloc( ).isSet( 
                 arg2wearloc( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, add_charmed, "(victim,time): очаровать victim на время time и добавить нам в последователи" )
{
    Character *victim;
    int duration;
    Affect af;
    RegisterList::const_iterator i;
    
    checkTarget( );
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    i = args.begin( );
    victim = arg2character( *i++ );
    duration = (i == args.end( ) ? -1 : i->toNumber( ));

    if (victim->master)
        follower_stop(victim);

    follower_add(victim, target);
    victim->leader = target;
    
    af.bitvector.setTable(&affect_flags);
    af.type      = gsn_charm_person;
    af.level     = target->getRealLevel( );
    af.duration  = duration;
    af.bitvector.setValue(AFF_CHARM);
    affect_to_char( victim, &af );

    // Ensure that when charm is off, default mobile AI won't be active for a while.
    if (victim->is_npc() && victim->getNPC()->behavior) {
        BasicMobileBehavior::Pointer bhv = victim->getNPC()->behavior.getDynamicPointer<BasicMobileBehavior>();
        if (bhv)
            bhv->setLastCharmTime();
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, add_pet, "(pet): добавить пета нам в последователи" )
{
    Character *pet;

    checkTarget( );
    CHK_NPC

    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );
    
    pet = arg2character( args.front( ) );
    if (!pet->is_npc( ))
        throw Scripting::Exception( "NPC field requested on PC" ); 
    
    if (pet->getNPC( )->behavior) {
        Pet::Pointer bhv = pet->getNPC( )->behavior.getDynamicPointer<Pet>( );
        if (bhv)
            bhv->config( target->getPC( ), pet->getNPC( ) );
    }

    if (pet->master)
        follower_stop(pet);

    affect_add_charm(pet);

    target->getPC( )->pet = pet->getNPC( );
    follower_add( pet, target );
    pet->leader = target;

    // Ensure that when charm is off, default mobile AI won't be active for a while.
    if (pet->getNPC()->behavior) {
        BasicMobileBehavior::Pointer bhv = pet->getNPC()->behavior.getDynamicPointer<BasicMobileBehavior>();
        if (bhv)
            bhv->setLastCharmTime();
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, look_auto, "(room): вывести описание комнаты room, будто там набрали look" )
{
    checkTarget( );
    do_look_auto( target, arg2room( get_unique_arg( args ) ) );
    return Register( );
}

NMI_GET(CharacterWrapper, screenreader, "пользуется ли персонаж клиентом или режимом для незрячих")
{
    checkTarget();
    return uses_screenreader(target);
}

NMI_GET( CharacterWrapper, affected, "список всех аффектов (List из структур Affect)" )
{
    checkTarget();
    RegList::Pointer rc(NEW);

    for (auto &paf: target->affected) 
        rc->push_back( AffectWrapper::wrap( *paf ) );
        
    return wrap(rc);
}

NMI_GET( CharacterWrapper, hasDestiny, "моб имеет предназначение (квестовые и спец-мобы)" )
{
    checkTarget( );
    CHK_PC
    
    if (target->getNPC( )->behavior)
        return target->getNPC( )->behavior->hasDestiny( );
    else
        return Register( false );
}

NMI_INVOKE( CharacterWrapper, hasOccupation, "(): моб имеет занятие (shopper,practicer,repairman,quest_trader,quest_master,healer,smithman,trainer,clanguard,adept)" )
{
    checkTarget( );
    CHK_PC

    DLString occName = args2string( args );
    return mob_has_occupation( target->getNPC( ), occName.c_str( ) );
}

NMI_INVOKE( CharacterWrapper, switchTo, "(mob): вселиться в тело моба" )
{
    Character *victim;
        
    checkTarget();
    CHK_NPC

    victim = arg2character( args.front( ) );
    if (!victim->is_npc( ))
        throw Scripting::Exception( "Impossible to switch to PC" ); 
    
    if (target->desc == 0)
        throw Scripting::Exception( "Zero descriptor for switch" ); 

    if (target->getPC( )->switchedTo) 
        throw Scripting::Exception( "Character already switched" ); 

    if (victim->desc != 0)
        throw Scripting::Exception( "Switch victim is already in use" ); 

    wiznet( WIZ_SWITCHES, WIZ_SECURE, target->get_trust( ), "%C1 switches into %C4.", target, victim );

    victim->getNPC( )->switchedFrom = target->getPC( );
    target->getPC( )->switchedTo = victim->getNPC( );
    
    target->desc->associate( victim );
    target->desc = 0;

    return Register( );
}


NMI_INVOKE( CharacterWrapper, switchFrom, "(): выселиться из моба обратно" )
{
    checkTarget( );
    CHK_PC
    
    if (target->desc == 0)
        throw Scripting::Exception( "Switched mobile has no descriptor" ); 
    
    if (!target->getNPC( )->switchedFrom) 
        throw Scripting::Exception( "Try to return from non-switched mobile" );

    wiznet( WIZ_SWITCHES, WIZ_SECURE, target->get_trust( ), "%C1 returns from %C2.", target->getNPC( )->switchedFrom, target );
    
    target->desc->associate( target->getNPC( )->switchedFrom );
    target->getNPC( )->switchedFrom->switchedTo = 0;
    target->getNPC( )->switchedFrom = 0;
    target->desc = 0;

    return Register( );
}

NMI_INVOKE( CharacterWrapper, setDead, "(): DEPRECATED" )
{
    checkTarget( );
    CHK_PC
    target->setDead( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, isDead, "(): DEPRECATED" )
{
    checkTarget( );
    CHK_PC
    return target->isDead( );
}

NMI_INVOKE( CharacterWrapper, writeWSCommand, "(cmd,args...): отправить в веб-клиент команду с аргументами" )
{
    checkTarget( );
    CHK_NPC

    Json::Value val;
    RegisterList::const_iterator i = args.begin( );

    val["command"] = (i++)->toString();

    for(int j=0;i != args.end();i++, j++) {
        switch(i->type) {
            case Register::NONE:
                val["args"][j] = Json::Value::null;
                break;
            case Register::NUMBER:
                val["args"][j] = i->toNumber();
                break;
            case Register::STRING:
                val["args"][j] = i->toString();
                break;
            default:
                throw Scripting::Exception( "Unsupported type exception" );
        }
    }

    return target->desc->writeWSCommand(val);
}

NMI_INVOKE( CharacterWrapper, eat, "(ob): заполнить желудок так, будто obj был съеден" )
{
    checkTarget( );
    ::Object *obj = arg2item( args.front( ) );

    if (obj->item_type == ITEM_FOOD) {
        desire_hunger->eat( target->getPC( ), obj->value0() * 2 );
        desire_full->eat( target->getPC( ), obj->value1() * 2 );
    }

    return Register( );
}

NMI_INVOKE( CharacterWrapper, drink, "(obj,amount): заполнить желудок так, будто от obj отхлебнули amount глотков" )
{
    checkTarget( );
    ::Object *obj;
    int amount;

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );

    obj = arg2item( args.front( ) );
    amount = args.back( ).toNumber( );

    if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOUNTAIN) {
        Liquid *liq = liquidManager->find( obj->value2() );

        desire_full->drink( target->getPC( ), amount, liq );
        desire_thirst->drink( target->getPC( ), amount, liq );
        desire_drunk->drink( target->getPC( ), amount, liq );
    }

    return Register( );
}

NMI_INVOKE(CharacterWrapper, give, "(vict,vnum|obj): дать персонажу vict предмет obj, создав его, если указан внум")
{
    checkTarget( );
    Character *vict = argnum2character(args, 1);
    Register arg2 = argnum(args, 2);
    ::Object *item;

    if (arg2.type == Register::NUMBER) {
        OBJ_INDEX_DATA *pObj = get_obj_index(arg2.toNumber());
        if (!pObj)
            throw Scripting::Exception("Object with this vnum does not exist.");

        item = create_object(pObj, 0);
    } else {
        item = arg2item(arg2);
    }

    obj_from_anywhere(item);
    obj_to_char(item, vict);

    vict->pecho("%^C1 дает тебе %O4.", target, item);
    vict->recho("%^C1 дает %C3 %O4.", target, vict, item);

    return Register();
}

NMI_INVOKE(CharacterWrapper, giveBack, "(vict,obj): вернуть персонажу vict предмет obj")
{
    checkTarget( );
    Character *vict = argnum2character(args, 1);
    ::Object *item = argnum2item(args, 2);

    if (item->carried_by != target)
        throw Scripting::Exception("Object you're trying to give back is not carried by this character.");
    
    obj_from_char(item);
    obj_to_char(item, vict);

    vict->pecho("%^C1 возвращает тебе %O4.", target, item);
    vict->recho("%^C1 возвращает %C3 %O4.", target, vict, item);

    return Register();
}

NMI_INVOKE(CharacterWrapper, attribute, "(name): вернуть аттрибут с данным именем, в виде строки или структуры, либо null")
{
    checkTarget();
    CHK_NPC
    DLString name = args2string(args);

    if (!target->getPC()->getAttributes().isAvailable(name))
        return Register();

    XMLAttribute::Pointer attr = target->getPC()->getAttributes().find(name)->second;
    return attr->toRegister();
}

NMI_INVOKE(CharacterWrapper, trustCheck, "(action, ch): выполнить проверку на траст для персонажа ch, вернет true если действие разрешено")
{
    checkTarget();
    CHK_NPC
    DLString action = argnum2string(args, 1);
    Character *ch = argnum2character(args, 2);
        
    XMLAttributeTrust::Pointer trust = target->getPC( )->getAttributes( ).findAttr<XMLAttributeTrust>( action );
    if (!trust)
        return false;

    return trust->check(ch);
}

NMI_INVOKE(CharacterWrapper, trustParse, "(action, trustArgs, successMsg): задать новый тип траста для действия action, вернет true если задано успешно")
{
    checkTarget();
    CHK_NPC
    DLString action = argnum2string(args, 1);
    DLString trustArgs = argnum2string(args, 2);
    DLString successMsg = argnum2string(args, 3);
    ostringstream buf;

    XMLAttributeTrust::Pointer trust = target->getPC( )->getAttributes( ).getAttr<XMLAttributeTrust>( action );
    
    bool rc = trust->parse(trustArgs, buf);
    if (rc)
        target->send_to(successMsg);
    target->pecho(buf.str());

    return rc;
}

NMI_INVOKE(CharacterWrapper, restring, "(skill,key,names,short,long,extra): установить аттрибут для рестринга результатов заклинаний")
{
    checkTarget( );
    CHK_NPC

    Skill *skill = argnum2skill(args, 1);
    DLString key = argnum2string(args, 2);
    DLString objName = argnum2string(args, 3);
    DLString objShort = argnum2string(args, 4);
    DLString objLong = argnum2string(args, 5);
    DLString objExtra = argnum2string(args, 6);

    XMLAttributeRestring::Pointer attr = target->getPC( )->getAttributes( ).getAttr<XMLAttributeRestring>(skill->getName());
    XMLAttributeRestring::iterator r = attr->find( key );
    if (r != attr->end( )) {
        r->second.name = objName;
        r->second.shortDescr = objShort;
        r->second.longDescr = objLong;
        r->second.description = objExtra;
    } else {
        (**attr)[key].name = objName;
        (**attr)[key].shortDescr = objShort;
        (**attr)[key].longDescr = objLong;
        (**attr)[key].description = objExtra;
    }

    target->getPC( )->save( );
    return Register( );
}

NMI_INVOKE(CharacterWrapper, hasBehavior, "(bhvName): true если среди поведений моба есть указанное")
{
    checkTarget();

    if (!target->is_npc())
        return false;

    DLString bhvName = args2string(args);
    Behavior *bhv = behaviorManager->findExisting(bhvName);
    if (!bhv)
        throw IllegalArgumentException();

    return Register(target->getNPC()->pIndexData->behaviors.isSet(bhv->getIndex()));
}

NMI_INVOKE(CharacterWrapper, behaviorMethod, "(methodName, args...): вызвать метод MobileBehavior с аргументами")
{
    checkTarget();
    CHK_PC
    DLString methodName = argnum2string(args, 1);

    if (methodName == "shot") {
        Character *attacker = argnum2character(args, 2);
        int door = argnum2number(args, 3);
        if (target->getNPC()->behavior)
            target->getNPC()->behavior->shot(attacker, door);
        return Register();
    }

    throw Scripting::Exception(methodName + " behavior method not supported yet");
}

NMI_INVOKE(CharacterWrapper, trigger, "(trigName, trigArgs...): вызвать триггер у персонажа или прототипа")
{
    checkTarget();

    // Get trig name such as "Death" or "Get", and trig arguments (all but first one)
    DLString trigName = argnum2string(args, 1);
    RegisterList trigArgs = args;
    trigArgs.pop_front();

    // If it's a mob, access its mob index data wrapper.
    WrapperBase *proto = target->is_npc() ? get_wrapper(target->getNPC()->pIndexData->wrapper) : 0;

    // Helper function will invoke onDeath, postDeath triggers on character and proto.
    Register result(false);
    fenia_trigger(result, trigName, trigArgs, this, proto);
    return result;
}

NMI_INVOKE(CharacterWrapper, menu, "([number, action]): очистить меню или установить пункт number с действием action")
{
    checkTarget( );
    CHK_NPC

    if (args.empty()) {
        target->getPC()->getAttributes().eraseAttribute("menu");
        return Register();
    }

    DLString number = argnum2string(args, 1);
    DLString action = argnum2string(args, 2);
    set_map_attribute_value(target->getPC(), "menu", number, action);
    return Register();
}

NMI_INVOKE(CharacterWrapper, hash, "(mod): вернуть ключ к хеш-таблице по модулю mod")
{
    checkTarget();
    int mod = args2number(args);
    return Register((int)(target->getID() % mod));
}
 
NMI_INVOKE( CharacterWrapper, api, "(): печатает этот api" )
{
    ostringstream buf;
    Scripting::traitsAPI<CharacterWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

NMI_GET(CharacterWrapper, killed, "статистика убийств мобов")
{
    checkTarget();
    CHK_NPC
    auto killingAttr = target->getPC()->getAttributes().getAttr<XMLKillingAttribute>("killed");
    return killingAttr->toRegister();
}

NMI_GET(CharacterWrapper, gquest, "статистика побед в глобальных квестах")
{
    checkTarget();
    CHK_NPC
    auto statAttr = target->getPC()->getAttributes().getAttr<XMLAttributeStatistic>("gquest");
    return statAttr->toRegister();
}

NMI_GET(CharacterWrapper, quest, "статистика побед в авто квестах")
{
    checkTarget();
    CHK_NPC
    auto statAttr = target->getPC()->getAttributes().getAttr<XMLAttributeStatistic>("questdata");
    return statAttr->toRegister();
}



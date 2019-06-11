/* $Id: characterwrapper.cpp,v 1.1.4.50.4.40 2009/11/08 17:46:27 rufina Exp $
 *
 * ruffina, 2004
 */

#include <iostream>

#include "json/json.h"

#include "logstream.h"
#include "mobilebehavior.h"
#include "mobilebehaviormanager.h"

#include "skill.h"
#include "skillmanager.h"
#include "clan.h"

#include "affect.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "desire.h"
#include "npcharacter.h"
#include "race.h"
#include "object.h"
#include "room.h"

#include "subprofession.h"
#include "occupations.h"
#include "interp.h"
#include "comm.h"
#include "save.h"
#include "mercdb.h"
#include "fight.h"
#include "magic.h"
#include "movement.h"
#include "act_move.h"
#include "merc.h"
#include "handler.h"
#include "alignment.h"
#include "wiznet.h"
#include "xmlattributecoder.h"
#include "xmlattributerestring.h"
#include "pet.h"
#include "recipeflags.h"

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

void CharacterWrapper::checkTarget( ) const throw( Scripting::Exception )
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

NMI_GET( CharacterWrapper, dead, "true, если персонаж полностью уничтожен (suicide/remort для pc, смерть для npc)" )
{
    return Register( zombie.getValue() );
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
        rc->push_back(wrap(obj));

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);
    return Register( obj );
}

GETWRAP( on, "объект, мебель, на которой сидим" )

GETWRAP( in_room, "комната, в которой сейчас находимся" ) 
GETWRAP( was_in_room, "комната, в которой находились перед закапыванием в могилу")
GETWRAP( mount, "на ком мы верхом или кто верхом на нас" )
    
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

NMI_GET( CharacterWrapper, alignMin, "название самого злого характера для расы и класса персонажа" )
{
    checkTarget( );
    CHK_NPC
    return align_min( target->getPC( ) );
}

NMI_GET( CharacterWrapper, alignMax, "название самого доброго характера для расы и класса персонажа" )
{
    checkTarget( );
    CHK_NPC
    return align_max( target->getPC( ) );
}

NMI_GET( CharacterWrapper, alignName, "название характера" )
{
    checkTarget( );
    return align_name( target );
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
INT_FIELD(timer, "сколько секунд прошло с последней команды")
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
INT_FIELD(add_affected_by, "расширение флагов аффектов (таблица .tables.affect_flags)")
INT_FIELD(detection, "флаги детектов (таблица .tables.detect_flags)")
INT_FIELD(position, "позиция (таблица .tables.position_table)")
INT_FIELD(carry_weight, "вес который несет чар")
INT_FIELD(carry_number, "количество вещей которое несет чар")
INT_FIELD(saving_throw, "савесы")
INT_FIELD(alignment, "характер, от -1000 до 1000")
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
INT_FIELD(anti_killed, "сколько жертв не моего align убито")
INT_FIELD(has_killed, "сколько жертв убито всего")
INT_FIELD(perm_hit, "max hp без шмота")
INT_FIELD(perm_mana, "max mana без шмота")
INT_FIELD(perm_move, "max move без шмота")
INT_FIELD(max_skill_points, "кол-во скилпоинтов у чара")
INT_FIELD(practice, "сколько практик")
INT_FIELD(train, "сколько тренировок")
INT_FIELD(loyalty, "лояльность по отношению к закону (рулеровскому)")
INT_FIELD(curse, "проклятье богов")
INT_FIELD(bless, "благословение богов")
INT_FIELD(bank_s, "серебра в банке")
INT_FIELD(bank_g, "золота в банке")
INT_FIELD(questpoints, "qp")
INT_FIELD(config, "настройки чара (таблица .tables.config_flags)")
INT_FIELD(shadow, "сколько висеть тени (shadowlife) в секундах")
INT_FIELD(start_room, "в какой комнате вошли в мир")
    
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

NMI_SET( CharacterWrapper, wearloc, "список слотов экипировки")
{
    checkTarget( );
    CHK_NPC
    target->getPC( )->wearloc.fromString( arg.toString( ) );
}

NMI_GET( CharacterWrapper, wearloc, "список слотов экипировки")
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->wearloc.toString();
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

NMI_GET( CharacterWrapper, lastAccessTime, "время последнего захода в мир" )
{
    checkTarget( );
    CHK_NPC
    return target->getPC( )->getLastAccessTime( ).getTimeAsString( );
}

NMI_GET( CharacterWrapper, profession, "класс (структура .Profession)" )
{
    checkTarget( );
    CHK_NPC
    return ProfessionWrapper::wrap( target->getPC( )->getProfession( )->getName( ) );
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

NMI_GET( CharacterWrapper, uniclass, "под-профессия универсала (.Profession)" )
{
    checkTarget( );
    CHK_NPC
    return ProfessionWrapper::wrap( target->getPC( )->getSubProfession( )->getName( ) );
}

NMI_SET( CharacterWrapper, uniclass, "под-профессия универсала (.Profession)" )
{
    checkTarget( );
    CHK_NPC
    if (arg.type == Register::NONE)
        target->getPC( )->setSubProfession( "none" );
    else
        target->getPC( )->setSubProfession( wrapper_cast<ProfessionWrapper>(arg)->name );
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
    CHK_NPC
    return ClanWrapper::wrap( target->getPC( )->getClan( )->getName( ) );
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

NMI_INVOKE( CharacterWrapper, say, "(format, args...): произносит вслух реплику, отформатированную как в методе act" )
{
    checkTarget( );
        
    DLString msg = regfmt(target, args).c_str();
    ::interpret_cmd(target, "say", msg.c_str());
    return Register();
}

NMI_INVOKE( CharacterWrapper, get_char_world, "(name): видимый для нас чар с именем name в мире" )
{
    checkTarget( );
    return wrap( ::get_char_world( target, args2string( args ) ) );
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
        throw Scripting::CustomException( "Invalid liquid name");

    list< ::Object *> drinks = ::get_objs_list_type(target, ITEM_DRINK_CON, target->carrying);
    for (list< ::Object *>::iterator o = drinks.begin(); o != drinks.end(); o++)
        if (liquidManager->find((*o)->value[2]) == liquid)
            return wrap(*o);

    return Register();
}

NMI_INVOKE( CharacterWrapper, get_recipe_carry, "(flag): вернет рецепт в инвентаре с заданным флагом (таблица .tables.recipe_flags)" )
{
    checkTarget( );

    bitstring_t flag = args2number(args);
    list< ::Object *> recipes = ::get_objs_list_type(target, ITEM_RECIPE, target->carrying);
    for (list< ::Object *>::iterator o = recipes.begin(); o != recipes.end(); o++)
        if (IS_SET((*o)->value[0], flag))
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

NMI_INVOKE( CharacterWrapper, get_char_room, "(name): поиск по имени видимого персонажа в той же комнате" )
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

NMI_INVOKE( CharacterWrapper, get_obj_carry, "(name): поиск по имени видимого объекта в экипировке или инвентаре" )
{
    checkTarget( );
    return wrap( ::get_obj_carry( target, args2string( args ) ) );
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
    
    target->send_to( regfmt(target, args) + "\r\n");
    
    return Register( );
}

NMI_INVOKE( CharacterWrapper, recho, "(fmt, args): выводит отформатированную строку всем в комнате, кроме нас" )
{
    checkTarget( );
    target->recho( regfmt( target, args ).c_str( ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, rvecho, "(vict, fmt, args...): выводит отформатированную строку всем в комнате, кроме нас и vict" )
{
    checkTarget( );
    RegisterList myArgs(args);
    
    Character *vict = args2character(args);
    myArgs.pop_front();

    target->recho( vict, regfmt( target, myArgs ).c_str( ) );
    return Register( );
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

NMI_INVOKE( CharacterWrapper, getClass, "(): строка с названием профессии" )
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
    target->add_follower( arg2character( get_unique_arg( args ) ) );
    return Register();
}

NMI_INVOKE( CharacterWrapper, stop_follower, "(): прекращает следование, снимает очарование" )
{
    checkTarget( );
    target->stop_follower();
    return Register();
}

NMI_INVOKE( CharacterWrapper, clearBehavior, "(): сбросить поведение моба до обычного" )
{
    checkTarget( );
    CHK_PC
    MobileBehaviorManager::assignBasic( target->getNPC( ) );
    return Register();
}


NMI_INVOKE( CharacterWrapper, get_random_room, "(): случайная комната, куда можно зайти" )
{
    checkTarget( );
    
    std::vector<Room *> rooms;
    Room *r;
    
    for (r = room_list; r; r = r->rnext)
        if (target->canEnter(r) && !r->isPrivate())
            rooms.push_back(r);
    
    if (rooms.empty())
        return Register( );
    else {
        r = rooms[::number_range(0, rooms.size() - 1)];
        return WrapperManager::getThis( )->getWrapper(r); 
    }
}

NMI_INVOKE( CharacterWrapper, is_safe, "(vict): защищают ли боги vict от нас" )
{
    checkTarget( );
    return ::is_safe_nomessage( target, 
                                arg2character( get_unique_arg( args ) ) );
}

NMI_INVOKE( CharacterWrapper, rawdamage, "(vict,dam,damtype): нанести vict повреждения в размере dam с типом damtype (таблица .tables.damage_table)" )
{
    RegisterList::const_iterator i;
    Character *victim;
    int dam;
    int dam_type = DAM_NONE;

    checkTarget( );

    if (args.size() < 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    victim = arg2character( *i );
    dam = (++i)->toNumber( );

    if (args.size() > 2) {
        DLString d = (++i)->toString();
        dam_type = damage_table.value( d.c_str(), true );
        if (dam_type == NO_FLAG)
            throw Scripting::CustomException( "Invalid damage type");
    }

    ::rawdamage(target, victim, dam_type, dam, true);

    return Register( );
}

NMI_INVOKE( CharacterWrapper, damage, "(vict,dam,skillName,damtype): нанести vict повреждения в размере dam умением skillName и типом damtype (таблица .tables.damage_table)" )
{
    RegisterList::const_iterator i;
    Character *victim;
    int dam;
    int dam_type = DAM_NONE;
    Skill *skill; 
    DLString skillName;

    checkTarget( );

    if (args.size() < 3)
       throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    victim = arg2character( *i );
    dam = (++i)->toNumber( );

    skillName = (++i)->toString( );
    skill = skillManager->findExisting( skillName );
    if (!skill)
        throw Scripting::CustomException( skillName + ": invalid skill name");
    
    if (args.size() > 2) {
        DLString d = (++i)->toString();
        dam_type = damage_table.value( d.c_str(), true );
        if (dam_type == NO_FLAG)
            throw Scripting::CustomException( "Invalid damage type");
    }

    ::damage(target, victim, dam, skill->getIndex( ), dam_type, true);

    return Register( );
}


NMI_INVOKE( CharacterWrapper, spell, "(skillName,level,vict,spellbane): скастовать на vict заклинание skillName уровне level, с возможным spellbane")
{
    RegisterList::const_iterator i;
    Skill *skill;
    Character *victim;
    int level;
    bool fBane;
    
    checkTarget( );

    if (args.size() < 4)
        throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    DLString d = i->toString();
    skill = SkillManager::getThis( )->findExisting( d.c_str( ) );
    
    i++;
    level = i->toNumber( );

    i++;
    victim = arg2character( *i );

    i++;
    fBane = i->toNumber( );

    if (!skill || !victim)
        throw Scripting::IllegalArgumentException( );
    
    spell( skill->getIndex( ), level, target, victim, fBane );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, multi_hit, "(vict): нанести один раунд повреждений жертве" )
{
    checkTarget( );
    ::multi_hit( target, arg2character( get_unique_arg( args ) ) );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, raw_kill, "([bodypart[,killer]]): убить. часть тела из таблицы .tables.part_flags или -1" )
{
    RegisterList::const_iterator i;
    Character *killer = NULL;
    int part = -1;

    checkTarget();
    
    i = args.begin( );

    if (i != args.end( )) {
        part = i->toNumber( );

        if (++i != args.end( ))
            killer = arg2character( *i );
    }
    
    raw_kill( target, part, killer, FKILL_CRY|FKILL_GHOST|FKILL_CORPSE );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectAdd, "(.Affect): повесить новый аффект" )
{
    checkTarget( );
    AffectWrapper *aw;
    Affect af;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    aw->toAffect( af );
    affect_to_char( target, &af );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectJoin, "(.Affect): повесить новый аффект или усилить существующий" )
{
    checkTarget( );
    AffectWrapper *aw;
    Affect af;

    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    aw = wrapper_cast<AffectWrapper>( args.front( ) );
    aw->toAffect( af );
    affect_join( target, &af );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, affectBitStrip, "(where,bit): снять все аффекты, устанавливающие в поле where (.tables.affwhere_flags) значение bit")
{
    int where, bits;
    
    checkTarget( );

    if (args.size( ) != 2)
        throw Scripting::NotEnoughArgumentsException( );
    
    where = args.front( ).toNumber( );
    bits = args.back( ).toNumber( );
    affect_bit_strip( target, where, bits );
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

NMI_INVOKE( CharacterWrapper, affectStrip, "(skillName): снять все аффекты с именем skillName" )
{
    checkTarget( );
    Skill *skill;
    
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    skill = skillManager->findExisting( args.front( ).toString( ) );
    
    if (!skill)
        throw Scripting::IllegalArgumentException( );
    
    affect_strip( target, skill->getIndex( ) );
    return Register( );
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

NMI_INVOKE( CharacterWrapper, addDarkShroud, "(): повесить темную ауру")
{
    Affect af;
    
    checkTarget( );

    af.where     = TO_AFFECTS;
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
    if (obj->getOwner( ))
        return false;
    if (obj->behavior)
        return false;
    if (!target->can_see( obj ))
        return false;
    if (obj->isAntiAligned( target ))
        return false;

    return true;
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
        target->println("Now you are mortal.");
        return 1;
    }
    else {
        target->getPC( )->getAttributes( ).getAttr<XMLAttributeCoder>( "coder" );
        target->getPC( )->setSecurity( 999 );
        target->println("Now you are immortal.");
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

NMI_INVOKE( CharacterWrapper, canRecall, "(): может ли прямо сейчас воспользоваться возвратом" )
{
    checkTarget( );

    if (IS_SET(target->in_room->room_flags, ROOM_NO_RECALL))
        return false; 
    if (IS_RAFFECTED(target->in_room, AFF_ROOM_CURSE))
        return false;
    if (IS_AFFECTED(target, AFF_CURSE))
        return false;
    if (target->isAffected(gsn_manacles))
        return false;
    if (target->position <= POS_SLEEPING || target->fighting)
        return false;

    return true;
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
        victim->stop_follower( );

    victim->add_follower( target );
    victim->leader = target;

    af.where     = TO_AFFECTS;
    af.type      = gsn_charm_person;
    af.level     = target->getRealLevel( );
    af.duration  = duration;
    af.bitvector = AFF_CHARM;
    affect_to_char( victim, &af );

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

    SET_BIT( pet->affected_by, AFF_CHARM );
    pet->add_follower( target );
    pet->leader = target;
    target->getPC( )->pet = pet->getNPC( );

    return Register( );
}

NMI_INVOKE( CharacterWrapper, look_auto, "(room): вывести описание комнаты room, будто там набрали look" )
{
    checkTarget( );
    do_look_auto( target, arg2room( get_unique_arg( args ) ) );
    return Register( );
}

NMI_GET( CharacterWrapper, affected, "список всех аффектов (List из структур Affect)" )
{
    checkTarget();
    RegList::Pointer rc(NEW);
    Affect *paf;

    for (paf = target->affected; paf != 0; paf = paf->next) 
        rc->push_back( AffectWrapper::wrap( *paf ) );
        
    Scripting::Object *sobj = &Scripting::Object::manager->allocate();
    sobj->setHandler(rc);

    return Register( sobj );
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

NMI_INVOKE( CharacterWrapper, hasOccupation, "(): моб имеет занятие (продавец,ремонтник,квестор)" )
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

NMI_INVOKE( CharacterWrapper, setDead, "(): пометить моба как умершего" )
{
    checkTarget( );
    CHK_PC
    target->setDead( );
    return Register( );
}

NMI_INVOKE( CharacterWrapper, isDead, "(): умер ли моб" )
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
        desire_hunger->eat( target->getPC( ), obj->value[0] * 2 );
        desire_full->eat( target->getPC( ), obj->value[1] * 2 );
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
        Liquid *liq = liquidManager->find( obj->value[2] );

        desire_full->drink( target->getPC( ), amount, liq );
        desire_thirst->drink( target->getPC( ), amount, liq );
        desire_drunk->drink( target->getPC( ), amount, liq );
    }

    return Register( );
}

NMI_INVOKE(CharacterWrapper, restring, "(skill,key,names,short,long): установить аттрибут для рестринга результатов заклинаний")
{
    checkTarget( );
    CHK_NPC
    if (args.size( ) != 5)
        throw Scripting::NotEnoughArgumentsException( );

    RegisterList::const_iterator i = args.begin( );
    DLString skillName = (i++)->toString();
    DLString key = (i++)->toString();
    DLString objName = (i++)->toString();
    DLString objShort = (i++)->toString();
    DLString objLong = (i)->toString();

    Skill *skill = skillManager->findExisting( skillName );
    if (!skill)
        throw Scripting::Exception( "Skill name not found" );

    XMLAttributeRestring::Pointer attr = target->getPC( )->getAttributes( ).getAttr<XMLAttributeRestring>( skillName );
    XMLAttributeRestring::iterator r = attr->find( key );
    if (r != attr->end( )) {
        r->second.name = objName;
        r->second.shortDescr = objShort;
        r->second.longDescr = objLong;
    } else {
        (**attr)[key].name = objName;
        (**attr)[key].shortDescr = objShort;
        (**attr)[key].longDescr = objLong;
    }

    target->getPC( )->save( );
    return Register( );
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


/* Dream Land, 2026 */
#include "questwrapper.h"
#include "feniaquest.h"
#include "questtargets.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "register-impl.h"
#include "reglist.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "room.h"
#include "integer.h"
#include "behavior.h"
#include "area.h"
#include "flagtable.h"

#include "merc.h"
#include "def.h"

using namespace Scripting;
using namespace std;

/*--------------------------------------------------------------------------
 * Argument helpers
 *
 * Deliberately local. The full set lives in feniaroot/wrap_utils.h, but
 * including that from here would make libquest_core need symbols from
 * libfeniaroot, which links libquest_core -- a cycle. These four are all this
 * file wants.
 *------------------------------------------------------------------------*/
static const Register & argnum( const RegisterList &args, int num )
{
    RegisterList::const_iterator a = args.begin( );

    for (int i = 1; i < num && a != args.end( ); i++)
        a++;

    if (a == args.end( ))
        throw Scripting::NotEnoughArgumentsException( );

    return *a;
}

static DLString argnum2string( const RegisterList &args, int num )
{
    return argnum( args, num ).toString( );
}

static int argnum2number( const RegisterList &args, int num )
{
    return argnum( args, num ).toNumber( );
}

/* Unwrapping, now that WrapperManagerBase carries all three forms. Each throws
 * when handed the wrong kind of value, which is correct: it happens inside an
 * NMI, and FeniaQuest::callType catches, croaks and reports. */
static Character * argnum2char( const RegisterList &args, int num )
{
    Character *ch = 0;

    FeniaManager::wrapperManager->getTarget( argnum( args, num ), ch );

    if (!ch)
        throw Scripting::Exception( "Argument is not a character" );

    return ch;
}

static NPCharacter * argnum2mob( const RegisterList &args, int num )
{
    Character *ch = argnum2char( args, num );

    if (!ch->is_npc( ))
        throw Scripting::Exception( "Quest targets can only be marked on mobiles" );

    return ch->getNPC( );
}

static ::Object * argnum2obj( const RegisterList &args, int num )
{
    ::Object *obj = 0;

    FeniaManager::wrapperManager->getTarget( argnum( args, num ), obj );

    if (!obj)
        throw Scripting::Exception( "Argument is not an object" );

    return obj;
}

static Room * argnum2room( const RegisterList &args, int num )
{
    Room *room = 0;

    FeniaManager::wrapperManager->getTarget( argnum( args, num ), room );

    if (!room)
        throw Scripting::Exception( "Argument is not a room" );

    return room;
}

/** Selection knobs, or the defaults when the argument is absent or null. */
static QuestSelectParams argnum2params( const RegisterList &args, int num )
{
    QuestSelectParams none;

    if ((int)args.size( ) < num)
        return none;

    const Register &reg = argnum( args, num );

    if (reg.type == Register::NONE)
        return none;

    if (reg.type != Register::OBJECT)
        throw Scripting::Exception( "Argument is not a quest selection, use quest.selection()" );

    Scripting::Object *obj = reg.toObject( );

    if (!obj || !obj->hasHandler( ))
        throw Scripting::Exception( "Argument is not a valid quest selection" );

    QuestSelectWrapper *sel = obj->getHandler( ).getDynamicPointer<QuestSelectWrapper>( );

    if (!sel)
        throw Scripting::Exception( "Argument is not a quest selection, use quest.selection()" );

    return sel->params;
}

static Register wrapEntity( Character *ch )
{
    if (!ch)
        return Register( );

    return FeniaManager::wrapperManager->getWrapper( ch );
}

static Register wrapEntity( ::Object *obj )
{
    if (!obj)
        return Register( );

    return FeniaManager::wrapperManager->getWrapper( obj );
}

static Register wrapEntity( Room *room )
{
    if (!room)
        return Register( );

    return FeniaManager::wrapperManager->getWrapper( room );
}

static Register wrapList( RegList::Pointer &list )
{
    Scripting::Object *listObj = &Scripting::Object::manager->allocate( );
    listObj->setHandler( list );
    return Register( listObj );
}

/*--------------------------------------------------------------------------
 * QuestWrapper
 *------------------------------------------------------------------------*/
NMI_INIT(QuestWrapper, "задание игрока")

QuestWrapper::QuestWrapper( ) : self( 0 )
{
}

QuestWrapper::QuestWrapper( FeniaQuest *quest ) : target( quest ), self( 0 )
{
}

Register QuestWrapper::wrap( FeniaQuest *quest )
{
    QuestWrapper::Pointer qw( NEW, quest );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( qw );

    return Register( sobj );
}

/** Recovered from the Fenia DB, or outliving the quest it pointed at, this
 *  answers nothing at all rather than reading freed memory. */
FeniaQuest * QuestWrapper::getTarget( )
{
    if (target.isEmpty( ))
        throw Scripting::Exception( "Quest is offline" );

    return target.getPointer( );
}

NMI_GET( QuestWrapper, typeName, "название типа задания, например KillQuest" )
{
    return Register( getTarget( )->getName( ) );
}

NMI_GET( QuestWrapper, charName, "имя игрока, взявшего задание" )
{
    return Register( getTarget( )->charName.getValue( ) );
}

NMI_GET( QuestWrapper, hero, "игрок, взявший задание, или null если он не в игре" )
{
    PCharacter *pch = getTarget( )->getHeroWorld( );

    if (!pch)
        return Register( );

    return FeniaManager::wrapperManager->getWrapper( (Character *)pch );
}

NMI_GET( QuestWrapper, state, "числовое состояние задания, см. QSTAT_* и свои значения типа" )
{
    return Register( getTarget( )->state.getValue( ) );
}

NMI_SET( QuestWrapper, state, "числовое состояние задания, см. QSTAT_* и свои значения типа" )
{
    getTarget( )->state.setValue( arg.toNumber( ) );
}

NMI_GET( QuestWrapper, hint, "сколько раз игрок просил подсказку" )
{
    return Register( getTarget( )->hint.getValue( ) );
}

NMI_SET( QuestWrapper, hint, "сколько раз игрок просил подсказку" )
{
    getTarget( )->hint.setValue( arg.toNumber( ) );
}

NMI_GET( QuestWrapper, timer, "сколько минут осталось на выполнение" )
{
    FeniaQuest *quest = getTarget( );
    PCMemoryInterface *pcm = quest->getHeroMemory( );

    if (!pcm)
        return Register( 0 );

    return Register( quest->getTime( pcm ) );
}

NMI_SET( QuestWrapper, timer, "сколько минут осталось на выполнение" )
{
    FeniaQuest *quest = getTarget( );
    PCMemoryInterface *pcm = quest->getHeroMemory( );

    if (!pcm)
        throw Scripting::Exception( "Quest hero not found, cannot set the timer" );

    quest->setTime( pcm, arg.toNumber( ) );
}

NMI_INVOKE( QuestWrapper, getVar, "(name): строковое значение переменной сценария, пустая строка если ее нет" )
{
    FeniaQuest *quest = getTarget( );
    DLString name = argnum2string( args, 1 );

    XMLMapBase<XMLString>::const_iterator v = quest->vars.find( name );

    if (v == quest->vars.end( ))
        return Register( DLString::emptyString );

    return Register( v->second.getValue( ) );
}

NMI_INVOKE( QuestWrapper, setVar, "(name, value): запомнить переменную сценария в пфайле игрока" )
{
    FeniaQuest *quest = getTarget( );
    DLString name = argnum2string( args, 1 );

    quest->vars[name].setValue( argnum2string( args, 2 ) );
    return Register( );
}

NMI_INVOKE( QuestWrapper, hasVar, "(name): есть ли такая переменная сценария" )
{
    FeniaQuest *quest = getTarget( );

    return Register( quest->vars.isAvailable( argnum2string( args, 1 ) ) );
}

NMI_INVOKE( QuestWrapper, eraseVar, "(name): забыть переменную сценария" )
{
    FeniaQuest *quest = getTarget( );

    quest->vars.erase( argnum2string( args, 1 ) );
    return Register( );
}

/* Numbers get their own pair rather than making scripts call toInt() on a
 * string: toInt() throws, and nothing on the quest path would catch it. */
NMI_INVOKE( QuestWrapper, getVarInt, "(name[, default]): числовое значение переменной сценария" )
{
    FeniaQuest *quest = getTarget( );
    DLString name = argnum2string( args, 1 );
    int fallback = args.size( ) > 1 ? argnum2number( args, 2 ) : 0;

    XMLMapBase<XMLString>::const_iterator v = quest->vars.find( name );

    if (v == quest->vars.end( ))
        return Register( fallback );

    Integer result;

    if (!Integer::tryParse( result, v->second.getValue( ) ))
        return Register( fallback );

    return Register( (int)result );
}

NMI_INVOKE( QuestWrapper, setVarInt, "(name, number): запомнить числовую переменную сценария" )
{
    FeniaQuest *quest = getTarget( );
    DLString name = argnum2string( args, 1 );

    quest->vars[name].setValue( DLString( argnum2number( args, 2 ) ) );
    return Register( );
}

NMI_INVOKE( QuestWrapper, varNames, "(): список (List) имен всех переменных сценария" )
{
    FeniaQuest *quest = getTarget( );
    RegList::Pointer list( NEW );

    for (XMLMapBase<XMLString>::const_iterator v = quest->vars.begin( ); v != quest->vars.end( ); v++)
        list->push_back( Register( v->first ) );

    return wrapList( list );
}

/* For eyeballing state in one line. A writable Map is deliberately not offered:
 * it would be a copy, and 'quest.vars.mode = 3' would look like it worked. */
NMI_INVOKE( QuestWrapper, varsString, "(): все переменные сценария одной строкой, для отладки" )
{
    FeniaQuest *quest = getTarget( );
    ostringstream buf;

    for (XMLMapBase<XMLString>::const_iterator v = quest->vars.begin( ); v != quest->vars.end( ); v++)
        buf << v->first << "=" << v->second.getValue( ) << " ";

    return Register( buf.str( ) );
}

NMI_INVOKE( QuestWrapper, wiznet, "(status[, text]): сообщить иммам о событии в задании" )
{
    FeniaQuest *quest = getTarget( );
    DLString status = argnum2string( args, 1 );

    if (args.size( ) > 1) {
        // %s and not the text itself: wiznet runs its argument through vfmt, and
        // a percent sign in scenario text would then eat whatever follows it.
        quest->wiznet( status.c_str( ), "%s", argnum2string( args, 2 ).c_str( ) );
    } else
        quest->wiznet( status.c_str( ) );

    return Register( );
}

NMI_INVOKE( QuestWrapper, scheduleDestroy, "(): снять задание с игрока в конце текущего такта" )
{
    getTarget( )->scheduleDestroy( );
    return Register( );
}

/* The five states the engine itself knows. Types are free to define their own
 * numbers past these -- kidnap and steal already do. */
NMI_GET( QuestWrapper, QSTAT_INIT, "состояние: задание только что создано" )
{
    return Register( (int)QSTAT_INIT );
}

NMI_GET( QuestWrapper, QSTAT_STARTED, "состояние: задание идет" )
{
    return Register( (int)QSTAT_STARTED );
}

NMI_GET( QuestWrapper, QSTAT_BROKEN_BY_HERO, "состояние: игрок сам испортил задание" )
{
    return Register( (int)QSTAT_BROKEN_BY_HERO );
}

NMI_GET( QuestWrapper, QSTAT_BROKEN_BY_OTHERS, "состояние: задание испортил кто-то другой" )
{
    return Register( (int)QSTAT_BROKEN_BY_OTHERS );
}

NMI_GET( QuestWrapper, QSTAT_FINISHED, "состояние: задание выполнено" )
{
    return Register( (int)QSTAT_FINISHED );
}

/*--------------------------------------------------------------------------
 * Target selection and marking
 *
 * The searches walk the whole mob or object list natively; only the conditions
 * come from the script. See questselectparams.h for why the knobs are so few.
 *------------------------------------------------------------------------*/
NMI_INVOKE( QuestWrapper, selection, "(): новый набор условий отбора, см. api у результата" )
{
    return QuestSelectWrapper::wrap( );
}

NMI_INVOKE( QuestWrapper, randomVictim, "(ch[, selection]): случайная подходящая жертва или null" )
{
    FeniaQuest *quest = getTarget( );
    Character *ch = argnum2char( args, 1 );

    if (ch->is_npc( ))
        throw Scripting::Exception( "randomVictim: hero must be a player" );

    return wrapEntity( quest->selectVictim( ch->getPC( ), argnum2params( args, 2 ) ) );
}

NMI_INVOKE( QuestWrapper, randomClient, "(ch[, selection]): случайный подходящий заказчик или null" )
{
    FeniaQuest *quest = getTarget( );
    Character *ch = argnum2char( args, 1 );

    if (ch->is_npc( ))
        throw Scripting::Exception( "randomClient: hero must be a player" );

    return wrapEntity( quest->selectClient( ch->getPC( ), argnum2params( args, 2 ) ) );
}

NMI_INVOKE( QuestWrapper, randomItem, "(ch[, selection]): случайный подходящий предмет в мире или null" )
{
    FeniaQuest *quest = getTarget( );
    Character *ch = argnum2char( args, 1 );

    if (ch->is_npc( ))
        throw Scripting::Exception( "randomItem: hero must be a player" );

    return wrapEntity( quest->selectItem( ch->getPC( ), argnum2params( args, 2 ) ) );
}

NMI_INVOKE( QuestWrapper, clientRoom, "(ch[, selection]): случайная комната, годная для заказчика, или null" )
{
    FeniaQuest *quest = getTarget( );
    Character *ch = argnum2char( args, 1 );

    if (ch->is_npc( ))
        throw Scripting::Exception( "clientRoom: hero must be a player" );

    return wrapEntity( quest->selectClientRoom( ch->getPC( ), argnum2params( args, 2 ) ) );
}

NMI_INVOKE( QuestWrapper, distantRoom, "(ch, from, range[, selection]): комната не ближе range от from, или null" )
{
    FeniaQuest *quest = getTarget( );
    Character *ch = argnum2char( args, 1 );

    if (ch->is_npc( ))
        throw Scripting::Exception( "distantRoom: hero must be a player" );

    return wrapEntity( quest->selectDistantRoom( ch->getPC( ),
                                                 argnum2room( args, 2 ),
                                                 argnum2number( args, 3 ),
                                                 argnum2params( args, 4 ) ) );
}

NMI_INVOKE( QuestWrapper, roomReachable, "(ch, room): доберется ли герой до комнаты" )
{
    FeniaQuest *quest = getTarget( );
    Character *ch = argnum2char( args, 1 );

    if (ch->is_npc( ))
        throw Scripting::Exception( "roomReachable: hero must be a player" );

    return Register( quest->isRoomReachable( ch->getPC( ), argnum2room( args, 2 ) ) );
}

NMI_INVOKE( QuestWrapper, markMob, "(mob, role): пометить моба целью задания; role -- victim, client, thief" )
{
    FeniaQuest *quest = getTarget( );

    quest->markMobile( argnum2mob( args, 1 ), argnum2string( args, 2 ) );
    return Register( );
}

NMI_INVOKE( QuestWrapper, markObj, "(obj, role[, mandatory]): пометить предмет целью; role -- loot, key; mandatory -- задание ломается, если предмет уничтожат" )
{
    FeniaQuest *quest = getTarget( );
    bool mandatory = args.size( ) > 2 && argnum( args, 3 ).toBoolean( );

    quest->markObject( argnum2obj( args, 1 ), argnum2string( args, 2 ), mandatory );
    return Register( );
}

NMI_INVOKE( QuestWrapper, findMob, "([role]): помеченный моб этого задания или null" )
{
    FeniaQuest *quest = getTarget( );
    DLString role;

    if (args.size( ) > 0)
        role = argnum2string( args, 1 );

    return wrapEntity( quest->findMarkedMobile( role ) );
}

NMI_INVOKE( QuestWrapper, findObj, "([role]): помеченный предмет этого задания или null" )
{
    FeniaQuest *quest = getTarget( );
    DLString role;

    if (args.size( ) > 0)
        role = argnum2string( args, 1 );

    return wrapEntity( quest->findMarkedObject( role ) );
}

NMI_INVOKE( QuestWrapper, clearTargets, "(): снять все метки этого задания, ничего не уничтожая" )
{
    getTarget( )->clearMarked( );
    return Register( );
}

/*--------------------------------------------------------------------------
 * QuestRewardWrapper
 *------------------------------------------------------------------------*/
NMI_INIT(QuestRewardWrapper, "награда за задание")

QuestRewardWrapper::QuestRewardWrapper( ) : self( 0 )
{
}

QuestRewardWrapper::QuestRewardWrapper( QuestReward::Pointer &reward )
                  : target( reward ), self( 0 )
{
}

Register QuestRewardWrapper::wrap( QuestReward::Pointer &reward )
{
    QuestRewardWrapper::Pointer rw( NEW, reward );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( rw );

    return Register( sobj );
}

QuestReward * QuestRewardWrapper::getTarget( )
{
    if (target.isEmpty( ))
        throw Scripting::Exception( "Quest reward is offline" );

    return target.getPointer( );
}

#define REWARD_FIELD(x, api) \
NMI_GET( QuestRewardWrapper, x, api ) \
{ \
    return Register( getTarget( )->x ); \
} \
NMI_SET( QuestRewardWrapper, x, api ) \
{ \
    getTarget( )->x = arg.toNumber( ); \
}

REWARD_FIELD(points, "сколько квестовых единиц выдать")
REWARD_FIELD(exp, "сколько опыта выдать вместо единиц по 'задание выполнить опыт'")
REWARD_FIELD(gold, "сколько золота выдать")
REWARD_FIELD(prac, "сколько практик выдать")
REWARD_FIELD(clanpoints, "сколько единиц уйдет в клан")
REWARD_FIELD(wordChance, "шанс в процентах получить слово силы")
REWARD_FIELD(scrollChance, "шанс в процентах получить свиток")

/*--------------------------------------------------------------------------
 * QuestSelectWrapper
 *------------------------------------------------------------------------*/
NMI_INIT(QuestSelectWrapper, "условия отбора цели для задания")

QuestSelectWrapper::QuestSelectWrapper( ) : self( 0 )
{
}

Register QuestSelectWrapper::wrap( )
{
    QuestSelectWrapper::Pointer sw( NEW );

    Scripting::Object *sobj = &Scripting::Object::manager->allocate( );
    sobj->setHandler( sw );

    return Register( sobj );
}

NMI_GET( QuestSelectWrapper, levelDiffMin, "нижняя граница разницы уровней цели и героя" )
{
    return Register( params.levelDiffMin );
}

NMI_SET( QuestSelectWrapper, levelDiffMin, "нижняя граница разницы уровней цели и героя" )
{
    params.levelDiffMin = arg.toNumber( );
    params.levelDiffSet = true;
}

NMI_GET( QuestSelectWrapper, levelDiffMax, "верхняя граница разницы уровней цели и героя" )
{
    return Register( params.levelDiffMax );
}

NMI_SET( QuestSelectWrapper, levelDiffMax, "верхняя граница разницы уровней цели и героя" )
{
    params.levelDiffMax = arg.toNumber( );
    params.levelDiffSet = true;
}

NMI_GET( QuestSelectWrapper, maxLevel, "потолок уровня самой цели, 0 если без потолка" )
{
    return Register( params.maxLevel );
}

NMI_SET( QuestSelectWrapper, maxLevel, "потолок уровня самой цели, 0 если без потолка" )
{
    params.maxLevel = arg.toNumber( );
}

NMI_GET( QuestSelectWrapper, noCaster, "не брать в цели заклинателей" )
{
    return Register( params.noCaster );
}

NMI_SET( QuestSelectWrapper, noCaster, "не брать в цели заклинателей" )
{
    params.noCaster = arg.toBoolean( );
}

NMI_GET( QuestSelectWrapper, visible, "цель должна быть видима герою прямо сейчас" )
{
    return Register( params.visible );
}

NMI_SET( QuestSelectWrapper, visible, "цель должна быть видима герою прямо сейчас" )
{
    params.visible = arg.toBoolean( );
}

NMI_GET( QuestSelectWrapper, carriedByNpc, "предмет должен быть в руках NPC, способного отдать его обратно" )
{
    return Register( params.carriedByNpc );
}

NMI_SET( QuestSelectWrapper, carriedByNpc, "предмет должен быть в руках NPC, способного отдать его обратно" )
{
    params.carriedByNpc = arg.toBoolean( );
}

NMI_INVOKE( QuestSelectWrapper, addVnum, "(vnum): добавить внум прототипа в список допустимых" )
{
    const Register &reg = argnum( args, 1 );

    params.vnums.insert( reg.toNumber( ) );
    return Register( );
}

NMI_GET( QuestSelectWrapper, vnums, "КОПИЯ списка допустимых внумов -- push_back по ней ничего не меняет, используй addVnum или присвоение" )
{
    RegList::Pointer list( NEW );

    for (std::set<int>::const_iterator v = params.vnums.begin( ); v != params.vnums.end( ); v++)
        list->push_back( Register( *v ) );

    return wrapList( list );
}

NMI_SET( QuestSelectWrapper, vnums, "заменить список допустимых внумов готовым List; собирай его целиком ДО присваивания" )
{
    params.vnums.clear( );

    if (arg.type == Register::NONE)
        return;

    if (arg.type != Register::OBJECT)
        throw Scripting::Exception( "vnums wants a List of vnums" );

    Scripting::Object *obj = arg.toObject( );

    if (!obj || !obj->hasHandler( ))
        throw Scripting::Exception( "vnums wants a List of vnums" );

    RegList *list = obj->getHandler( ).getDynamicPointer<RegList>( );

    if (!list)
        throw Scripting::Exception( "vnums wants a List of vnums" );

    for (RegList::const_iterator v = list->begin( ); v != list->end( ); v++)
        params.vnums.insert( v->toNumber( ) );
}

NMI_GET( QuestSelectWrapper, requireActFlag, "act-флаг, который обязан быть у цели, можно несколько через пробел (см. .tables.act_flags)" )
{
    if (params.requireActFlag <= 0)
        return Register( DLString::emptyString );

    return Register( act_flags.names( params.requireActFlag ) );
}

NMI_SET( QuestSelectWrapper, requireActFlag, "act-флаг, который обязан быть у цели, можно несколько через пробел (см. .tables.act_flags)" )
{
    DLString name = arg.toString( );

    if (name.empty( )) {
        params.requireActFlag = 0;
        return;
    }

    bitstring_t bit = act_flags.bitstring( name, false );

    // NO_FLAG, not 0. An unknown name answers -99, and -99 used as a mask has
    // bit 0 set -- which is ACT_IS_NPC, carried by every mob alive -- so a typo
    // would have turned this filter into "match everything" while the getter
    // read the knob back as unset, because names() maps NO_FLAG to the empty
    // string. Silent in both directions, which is the one thing this typed
    // wrapper exists to prevent.
    if (bit == NO_FLAG || bit <= 0)
        throw Scripting::Exception( "No such act flag: " + name );

    params.requireActFlag = bit;
}

NMI_GET( QuestSelectWrapper, requireBehavior, "поведение, которое обязано быть у цели; вместе с requireActFlag достаточно ЛЮБОГО из них" )
{
    return Register( params.requireBehavior );
}

NMI_SET( QuestSelectWrapper, requireBehavior, "поведение, которое обязано быть у цели; вместе с requireActFlag достаточно ЛЮБОГО из них" )
{
    DLString name = arg.toString( );

    if (!name.empty( ) && !behaviorManager->findExisting( name ))
        throw Scripting::Exception( "No such behavior: " + name );

    params.requireBehavior = name;
}

NMI_GET( QuestSelectWrapper, roomNoCast, "не брать комнаты с флагом ROOM_NO_CAST" )
{
    return Register( params.roomNoCast );
}

NMI_SET( QuestSelectWrapper, roomNoCast, "не брать комнаты с флагом ROOM_NO_CAST" )
{
    params.roomNoCast = arg.toBoolean( );
}

NMI_GET( QuestSelectWrapper, excludeAreaName, "русское название зоны, комнаты которой не берем" )
{
    return Register( params.excludeAreaName );
}

NMI_SET( QuestSelectWrapper, excludeAreaName, "русское название зоны, комнаты которой не берем" )
{
    DLString name = arg.toString( );

    // Area names are enumerable, so a wrong one is checkable, and unchecked it
    // would be the third silent no-op in this class: a name that matches nothing
    // simply excludes nothing.
    if (!name.empty( )) {
        bool found = false;

        for (AreaIndexVector::const_iterator a = areaIndexes.begin( );
             a != areaIndexes.end( ) && !found; a++)
            if ((*a)->getName( ) == name)
                found = true;

        if (!found)
            throw Scripting::Exception( "No area named: " + name );
    }

    params.excludeAreaName = name;
}

NMI_GET( QuestSelectWrapper, noBehaviorInHometown, "имя поведения, которое не берем в цели внутри родных городов" )
{
    return Register( params.noBehaviorInHometown );
}

NMI_SET( QuestSelectWrapper, noBehaviorInHometown, "имя поведения, которое не берем в цели внутри родных городов" )
{
    DLString name = arg.toString( );

    // Checked here rather than in the search: a name nobody recognizes makes the
    // filter a silent no-op, and "cityGuard" for "cityguard" would quietly put
    // the town watch back on the menu. The typed wrapper exists so a wrong KNOB
    // is loud; a wrong VALUE should be too.
    if (!name.empty( ) && !behaviorManager->findExisting( name ))
        throw Scripting::Exception( "No such behavior: " + name );

    params.noBehaviorInHometown = name;
}

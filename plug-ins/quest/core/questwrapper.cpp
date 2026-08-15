/* Dream Land, 2026 */
#include "questwrapper.h"
#include "feniaquest.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "register-impl.h"
#include "reglist.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "integer.h"

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

#include "logstream.h"
#include "merc.h"

#include "autoquestwrapper.h"
#include "questregistrator.h"
#include "questscenario.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"

#include "def.h"
#include "lang.h"

using Scripting::NativeTraits;

NMI_INIT(AutoQuestWrapper, "тип автоквеста")

AutoQuestWrapper::AutoQuestWrapper( ) : target( NULL )
{
}

void
AutoQuestWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "AutoQuest wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void AutoQuestWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
AutoQuestWrapper::setTarget( QuestRegistratorBase *reg )
{
    target = reg;
    id = reg->getID();
}

void
AutoQuestWrapper::checkTarget( ) const
{
    if (zombie.getValue())
        throw Scripting::Exception( "AutoQuest type is dead" );

    if (!target)
        throw Scripting::Exception( "AutoQuest type is offline" );
}

QuestRegistratorBase *
AutoQuestWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( AutoQuestWrapper, name, "имя типа квеста, например KillQuest" )
{
    checkTarget( );
    return Register( target->getName( ) );
}

NMI_GET( AutoQuestWrapper, priority, "вес типа при случайном выборе задания" )
{
    checkTarget( );
    return Register( target->getPriority( ) );
}

NMI_GET( AutoQuestWrapper, minAutoLevel, "мин. уровень для выдачи без реморта" )
{
    checkTarget( );
    return Register( target->getMinAutoLevel( ) );
}

NMI_GET( AutoQuestWrapper, shortDescr, "название задания на языке по умолчанию" )
{
    checkTarget( );
    return Register( target->getShortDescr( LANG_DEFAULT ) );
}

NMI_INVOKE( AutoQuestWrapper, getShortDescr, "(lang): название задания на языке lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    return Register( target->getShortDescr( argnum2lang( args, 1 ) ) );
}

NMI_GET( AutoQuestWrapper, difficulty, "сложность задания на языке по умолчанию" )
{
    checkTarget( );
    return Register( target->getDifficulty( LANG_DEFAULT ) );
}

NMI_INVOKE( AutoQuestWrapper, getDifficulty, "(lang): сложность на языке lang (0=en,1=ru,2=ua)" )
{
    checkTarget( );
    return Register( target->getDifficulty( argnum2lang( args, 1 ) ) );
}

NMI_GET( AutoQuestWrapper, scenarios, "список (List) имен сценариев, пустой если их нет" )
{
    checkTarget( );
    RegList::Pointer list(NEW);

    // Only some quest types are scenario-driven; the rest have no such base and
    // legitimately answer with an empty list rather than an error.
    const QuestScenariosContainer *container
        = dynamic_cast<const QuestScenariosContainer *>( target );

    if (container) {
        StringList names = container->getScenarioNames( );

        for (StringList::const_iterator n = names.begin( ); n != names.end( ); n++)
            list->push_back( Register( *n ) );
    }

    return ::wrap(list);
}

NMI_INVOKE( AutoQuestWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<AutoQuestWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AutoQuestWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AutoQuestWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

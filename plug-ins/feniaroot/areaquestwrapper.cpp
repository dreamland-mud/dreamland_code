#include "logstream.h"
#include "object.h"
#include "merc.h"

#include "loadsave.h"

#include "areaquestwrapper.h"
#include "areaquestutils.h"
#include "xmlattributeareaquest.h"
#include "structwrappers.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "register-impl.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "pcharacter.h"

#include "def.h"

using Scripting::NativeTraits;

NMI_INIT(AreaQuestWrapper, "квест в арии")

AreaQuestWrapper::AreaQuestWrapper( ) : target( NULL )
{
}

void 
AreaQuestWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "AreaQuest wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void AreaQuestWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void
AreaQuestWrapper::setTarget( AreaQuest * quest )
{
    target = quest;
    id = quest->getID();
}

void 
AreaQuestWrapper::checkTarget( ) const 
{
    if (zombie.getValue())
        throw Scripting::Exception( "AreaQuest is dead" );

    if (!target)
        throw Scripting::Exception( "AreaQuest is offline");
}

AreaQuest *
AreaQuestWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

NMI_GET( AreaQuestWrapper, title, "название квеста") 
{ 
    checkTarget( ); 
    return Register( target->title );
}

NMI_GET( AreaQuestWrapper, description, "описание квеста") 
{ 
    checkTarget( ); 
    return Register( target->description );
}

NMI_GET( AreaQuestWrapper, vnum, "номер квеста") 
{ 
    checkTarget( ); 
    return Register( target->vnum );
}

NMI_GET( AreaQuestWrapper, minLevel, "мин. уровень, на котором выдают квест") 
{ 
    checkTarget( ); 
    return Register( target->minLevel );
}

NMI_GET( AreaQuestWrapper, maxLevel, "макс. уровень, на котором выдают квест") 
{ 
    checkTarget( ); 
    return Register( target->maxLevel );
}

NMI_GET( AreaQuestWrapper, limitPerLife, "сколько раз за реморт можно выполнить") 
{ 
    checkTarget( ); 
    return Register( target->limitPerLife );
}

NMI_GET( AreaQuestWrapper, oncePerDay, "есть ли ограничение не чаще раза в сутки") 
{ 
    checkTarget( ); 
    return Register( target->oncePerDay );
}

NMI_GET( AreaQuestWrapper, align, "разрешенные натуры или 0 (таблица .tables.align_flags)" ) 
{
    checkTarget( ); 
    return Register((int)target->align.getValue());
}

NMI_GET( AreaQuestWrapper, classes, "разрешенные классы или пустая строка (olchelp class)" ) 
{
    checkTarget();
    return Register(target->classes.toString());
}

NMI_GET( AreaQuestWrapper, hometowns, "разрешенные домашние города или пустая строка" ) 
{
    checkTarget();
    return Register(target->hometowns.toString());
}


NMI_GET( AreaQuestWrapper, flags, "флаги квеста (таблица .tables.areaquest_flags)" ) 
{
    checkTarget();
    return Register((int)target->flags);
}

NMI_GET( AreaQuestWrapper, area, "зона (AreaIndex), в которой объявлен квест" ) 
{
    checkTarget();
    return WrapperManager::getThis()->getWrapper(target->pAreaIndex);
}

static int args2queststep(AreaQuest *q, const RegisterList &args)
{
    int step = argnum2number(args, 1);
    if (step < 0 || step > q->steps.size())
        throw Scripting::IndexOutOfBoundsException();
    return step;
}

NMI_INVOKE( AreaQuestWrapper, step, "(num): структура шага квеста по номеру" ) 
{
    checkTarget();
    int step = args2queststep(target, args);
    return target->steps[step]->toRegister();
}

NMI_INVOKE( AreaQuestWrapper, data, "(ch): статус этого квеста для ch (из аттрибута areaquest)" ) 
{
    checkTarget();
    PCharacter *ch = argnum2player(args, 1);
    AreaQuestData &qdata = aquest_data(ch, target->vnum.toString());
    return qdata.toRegister();
}

/** Invoke onCancel trigger for quest that can be responsible for cleaning up interim items etc. */
static void aqprog_cancel(PCharacter *ch, AreaQuest *q)
{
    FENIA_VOID_CALL(q, "Cancel", "C", ch);
}

NMI_INVOKE( AreaQuestWrapper, cancel, "(ch): отменить этот квест для ch, вернет true если квест был запущен" ) 
{
    checkTarget();
    PCharacter *ch = argnum2player(args, 1);

    AreaQuestData &qdata = aquest_data(ch, target->vnum.toString());
    
    if (!qdata.questActive())
        return Register(false);

    aqprog_cancel(ch, target);

    qdata.cancel();

    return Register(true);
}

NMI_INVOKE( AreaQuestWrapper, rollback, "(ch): откатить этот квест на предыдущий шаг для ch, вернет новый номер шага" ) 
{
    checkTarget();
    PCharacter *ch = argnum2player(args, 1);
    AreaQuestData &qdata = aquest_data(ch, target->vnum.toString());
    qdata.rollback();
    return Register(qdata.step);
}

NMI_INVOKE( AreaQuestWrapper, canParticipate, "(ch): персонаж ch удовлетряет всем условиям для начала квеста" ) 
{
    checkTarget();
    PCharacter *ch = argnum2player(args, 1);
    AreaQuestData &qdata = aquest_data(ch, target->vnum.toString());
    return Register(aquest_can_participate(ch, target, qdata));
}

NMI_INVOKE( AreaQuestWrapper, api, "(): печатает этот API" )
{
    ostringstream buf;
    Scripting::traitsAPI<AreaQuestWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AreaQuestWrapper, rtapi, "(): печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( AreaQuestWrapper, clear, "(): очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

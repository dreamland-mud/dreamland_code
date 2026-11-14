#include "logstream.h"
#include "merc.h"

#include "autoquestwrapper.h"
#include "questregistrator.h"
#include "questexceptions.h"
#include "feniaquest.h"
#include "questwrapper.h"
#include "xmlattributequestdata.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "wrappermanager.h"
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

/* A quest handed out by someone other than a questor: a cult adept's rite.
 * Bypasses isApplicable on purpose -- such a type answers false there so the
 * questor never rolls it -- and leaves every "may this player have a quest"
 * rule (cooldown, confirmation, the giver's own mood) to the calling script,
 * which knows its giver. The one rule kept here is the one nobody may skip:
 * a player holds a single quest. */
NMI_INVOKE( AutoQuestWrapper, start, "(ch, giver): выдать игроку ch задание этого типа от моба giver (onCreate получит giver вместо квестора); задание игрока или null, если у ch уже есть задание или сценарий отказал (тогда 1-3 минуты ожидания, как у квестора)" )
{
    QuestRegistratorBase *reg = getTarget( );
    PCharacter *pch = argnum2player( args, 1 );
    NPCharacter *giver = argnum2mobile( args, 2 );
    XMLAttributes &attributes = pch->getAttributes( );

    // isAvailable, not findAttr: a quest node that failed to load still
    // occupies the slot, exactly as the questor sees it.
    if (attributes.isAvailable( "quest" ))
        return Register( );

    Quest::Pointer quest;

    try {
        quest = reg->createQuest( pch, giver );
    } catch (const QuestCannotStartException &) {
        // The questor's delay_noquest: a refusal costs a short wait, so a
        // giver asked again and again does not walk the world each time.
        attributes.getAttr<XMLAttributeQuestData>( "questdata" )->setTime( number_range( 1, 3 ) );
        return Register( );
    }

    attributes.addAttribute( quest, "quest" );
    attributes.getAttr<XMLAttributeQuestData>( "questdata" )->setStartTime( );
    PCharacterManager::save( pch );

    FeniaQuest *fq = quest.getDynamicPointer<FeniaQuest>( );

    if (!fq)
        return Register( );

    return QuestWrapper::wrap( fq );
}

/* The way back in for a giver that is not a questor: the adept has to find the
 * rite it handed out when the hero returns, and nothing else holds it. */
NMI_INVOKE( AutoQuestWrapper, questOf, "(ch): текущее задание игрока ch, если оно этого типа, иначе null" )
{
    QuestRegistratorBase *reg = getTarget( );
    Character *ch = argnum2character( args, 1 );

    if (ch->is_npc( ))
        return Register( );

    FeniaQuest::Pointer quest = ch->getPC( )->getAttributes( ).findAttr<FeniaQuest>( "quest" );

    if (!quest || quest->getName( ) != reg->getName( ))
        return Register( );

    return QuestWrapper::wrap( quest.getPointer( ) );
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

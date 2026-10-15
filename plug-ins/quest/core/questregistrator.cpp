/* $Id$
 *
 * ruffina, 2004
 */
#include "questregistrator.h"
#include "questexceptions.h"
#include "feniaquest.h"
#include "pcharacter.h"
#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "logstream.h"
#include "fenia/exceptions.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

long long QuestRegistratorBase::getID( ) const
{
    if (feniaId <= 0)
        throw Scripting::Exception( getName( ) + ": feniaId not set in the quest config" );

    return (feniaId.getValue( ) << 4) | 11;
}

void QuestRegistratorBase::linkFeniaWrapper( )
{
    if (!FeniaManager::wrapperManager) {
        LogStream::sendError( )
            << "No Fenia manager when linking quest type " << getName( )
            << " -- its Fenia triggers will never fire" << endl;
        return;
    }

    // A type with no feniaId is a config omission, not a reason to abort the
    // boot: it simply cannot carry Fenia logic until someone gives it an id.
    try {
        FeniaManager::wrapperManager->linkWrapper( this );
    } catch (const Scripting::Exception &e) {
        LogStream::sendError( )
            << "Cannot link Fenia wrapper for quest type " << getName( )
            << ": " << e.what( ) << endl;
    }
}

/** Detaching is only safe while feniaroot is still up, and by unload time it
 *  usually is not.
 *
 *  Because feniaroot links libquest_core, the unload cascade takes feniaroot
 *  down FIRST. Its backup() clears every dynamic handler but leaves our
 *  'wrapper' pointer non-null, so WrapperTarget::getWrapper() -- which resolves
 *  the pointer through hasHandler() -- returns NULL, and extractWrapper() would
 *  then make a virtual call on it. That is a segfault in every graceful shutdown
 *  and every `plug reload all`, armed the moment one autoquest wrapper exists.
 *  Same guard as wrappedcommand.cpp:34 and the three skills_impl extract sites.
 */
void QuestRegistratorBase::unlinkFeniaWrapper( )
{
    if (!FeniaManager::wrapperManager)
        return;

    extractWrapper( false );
}

int QuestRegistratorBase::getFeniaId( ) const
{
    return feniaId.getValue( );
}

bool QuestRegistratorBase::needsFeniaApplicable( ) const
{
    return needsApplicable.getValue( );
}

Quest::Pointer QuestRegistratorBase::createQuest( PCharacter *pch, NPCharacter *questor ) const
{
    FeniaQuest::Pointer quest( NEW );

    // The TYPE name, which is also the config file's name and the key every
    // victory tally is counted under. Set before create(), because create() is
    // what looks the .AutoQuest() wrapper up by it.
    quest->setTypeName( getName( ) );
    quest->create( pch, questor );
    return quest;
}

bool QuestRegistratorBase::isApplicable( PCharacter *pch, bool fAuto ) const
{
    bool answer;

    if (fAuto && pch->getRemorts().size() == 0 && pch->getRealLevel() < minAutoLevel)
        return false;

    if (askFeniaApplicable( pch, fAuto, answer ))
        return answer;

    // The miss is logged where it happens: croak for a script error, the
    // boot check in QuestTypesPlugin for a hook that is not bound at all.
    return !needsFeniaApplicable( );
}

bool QuestRegistratorBase::hasFeniaApplicable( ) const
{
    if (!FeniaManager::wrapperManager)
        return false;

    WrapperBase *wrapper = const_cast<QuestRegistratorBase *>( this )->getWrapper( );
    Register method;

    return wrapper && wrapper->triggerFunction( Scripting::IdRef( "onApplicable" ), method );
}

bool QuestRegistratorBase::askFeniaApplicable( PCharacter *pch, bool fAuto, bool &answer ) const
{
    if (!FeniaManager::wrapperManager)
        return false;

    WrapperBase *wrapper = const_cast<QuestRegistratorBase *>( this )->getWrapper( );

    if (!wrapper)
        return false;

    Scripting::IdRef methodId( "onApplicable" );
    Register method;

    if (!wrapper->triggerFunction( methodId, method ))
        return false;

    RegisterList args;
    args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)pch ) );
    args.push_back( Register( (int)fAuto ) );

    Register rc;

    try {
        rc = method.toFunction( )->invoke( Register( wrapper->getSelf( ) ), args );
    } catch (const ::Exception &e) {
        // Croak so the immortals see the backtrace, then treat it as no answer
        // (isApplicable decides by <needsApplicable>).
        FeniaManager::getThis( )->croak( wrapper, methodId, e );
        return false;
    }

    if (rc.type == Register::NUMBER || rc.type == Register::STRING) {
        answer = rc.toBoolean( );
        return true;
    }

    if (rc.type != Register::NONE)
        LogStream::sendError( ) << "Quest type " << getName( ) << ": onApplicable answered "
                                << rc.getTypeName( ) << " where true or false was expected" << endl;

    return false;
}

const DLString& QuestRegistratorBase::getName( ) const
{
    return name;
}

void QuestRegistratorBase::setName( const DLString &n )
{
    name = n;
}

int QuestRegistratorBase::getPriority( ) const
{
    return priority.getValue( );
}

int QuestRegistratorBase::getMinAutoLevel( ) const
{
    return minAutoLevel.getValue( );
}

const DLString& QuestRegistratorBase::getShortDescr( lang_t lang ) const
{
    return shortDesc.getForLang( lang );
}

const DLString& QuestRegistratorBase::getDifficulty( lang_t lang ) const
{
    return difficulty.getForLang( lang );
}

/* Input matching stays language-blind on purpose. A player who has typed
 * 'задание просить убийство' for years must keep it after the English and
 * Ukrainian names arrive, whatever language they read the menu in --
 * matchesUnstrict walks every slot, so the new names only add ways in. */
bool QuestRegistratorBase::matchesShortDescr( const DLString &arg ) const
{
    return shortDesc.matchesUnstrict( arg );
}


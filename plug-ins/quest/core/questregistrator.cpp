/* $Id$
 *
 * ruffina, 2004
 */
#include "questregistrator.h"
#include "questexceptions.h"
#include "pcharacter.h"
#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "logstream.h"
#include "fenia/exceptions.h"

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

bool QuestRegistratorBase::applicable( PCharacter *pch, bool fAuto ) const
{
    if (fAuto && pch->getRemorts().size() == 0 && pch->getRealLevel() < minAutoLevel)
        return false;

    return true;
}

const DLString& QuestRegistratorBase::getName( ) const
{
    return getType( );
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


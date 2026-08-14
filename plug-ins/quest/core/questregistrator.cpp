/* $Id$
 *
 * ruffina, 2004
 */
#include "questregistrator.h"
#include "questexceptions.h"
#include "pcharacter.h"


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


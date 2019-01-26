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

const DLString& QuestRegistratorBase::getShortDescr( ) const
{
    return shortDesc;
}

const DLString& QuestRegistratorBase::getDifficulty( ) const
{
    return difficulty;
}


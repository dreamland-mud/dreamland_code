/* $Id$
 *
 * ruffina, 2004
 */
#include "questregistrator.h"


bool QuestRegistratorBase::applicable( PCharacter * ) const
{
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


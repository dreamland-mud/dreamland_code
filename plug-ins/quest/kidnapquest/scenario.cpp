/* $Id: scenario.cpp,v 1.1.2.9.18.3 2007/09/29 19:34:03 rufina Exp $
 *
 * ruffina, 2004
 */
#include "scenario.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "msgformatter.h"
#include "object.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"
#include "def.h"

RACE(none);

/*
 * KSPrinceData
 */
void KSPrinceData::dress( NPCharacter *mob, NPCharacter *king ) const 
{
    QuestMobileAppearence::dress( mob );

    if (mob->getRace( ) == race_none)
        mob->setRace( king->getRace( )->getName( ) );
}


/*
 * KidnapScenario
 */
void KidnapScenario::onQuestStart( PCharacter *hero, NPCharacter *questman, NPCharacter *king ) const
{
    tell_raw( hero, questman, 
              "У {W%s{G случилось несчастье. Срочно требуется твоя помощь.",
                   king->getNameP( '2' ).c_str() );
    tell_raw( hero, questman, 
             "Ищи %s в местности под названием {W%s{G ({W{hh%s{hx{G).",
                   GET_SEX(king, "его", "его", "ее"), king->in_room->getName(), king->in_room->areaName().c_str() );
}


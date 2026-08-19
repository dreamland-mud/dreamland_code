/* $Id: questring.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 *
 * Stat formulas moved to Fenia (.tmp.questreward, family "ring"); the shared
 * level-stamp + affect plumbing lives in PersonalQuestReward::equip.
 */

#include "questring.h"
#include "pcharacter.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

void QuestRing::wear( Character *ch )
{
    ch->pecho(_("{CТвое кольцо ярко вспыхивает.{x"));
}

DLString QuestRing::questFamily( ) const
{
    return "ring";
}

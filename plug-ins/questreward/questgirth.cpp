/* $Id: questgirth.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 *
 * The stat formulas moved to Fenia (.tmp.questreward, family "girth"); the
 * shared level-stamp + affect plumbing lives in PersonalQuestReward::equip.
 * All that stays class-specific is the wear flash and the family name.
 */

#include "questgirth.h"
#include "pcharacter.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

void QuestGirth::wear( Character *ch )
{
    ch->pecho(_("{CТвой пояс ярко вспыхивает.{x"));
}

DLString QuestGirth::questFamily( ) const
{
    return "girth";
}

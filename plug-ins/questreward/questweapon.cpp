/* $Id: questweapon.cpp,v 1.1.2.10.6.3 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 *
 * The weapon runs the shared PersonalQuestReward::equip: Fenia
 * (.tmp.questreward, family "weapon") shapes the dice and flags and answers
 * every affect, including the weapon class and attack overrides.
 */

#include "questweapon.h"
#include "character.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

void QuestWeapon::wear(Character *ch)
{
    ch->pecho(_("{CТвое оружие ярко вспыхивает.{x"));
}

DLString QuestWeapon::questFamily( ) const
{
    return "weapon";
}

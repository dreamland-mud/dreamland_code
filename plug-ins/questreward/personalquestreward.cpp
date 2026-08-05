/* $Id: personalquestreward.cpp,v 1.1.2.10.6.3 2008/03/21 22:41:58 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */
#include "personalquestreward.h"
#include "class.h"
#include "pcharacter.h"
#include "core/object.h"
#include "act.h"
#include "loadsave.h"

#include "def.h"
#include "l10n.h"

void PersonalQuestReward::get( Character *ch ) 
{ 
    if (!canEquip( ch ))
        return;

    oldact_p(_("{BМерцающая аура окружает $o4.\n\r{x"), ch, obj, 0, TO_CHAR, POS_SLEEPING);
}


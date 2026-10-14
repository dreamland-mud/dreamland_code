/* $Id: train.cpp,v 1.1.2.2.6.10 2010-09-01 21:20:45 rufina Exp $
 *
 * ruffina, 2004
 */

#include "train.h"

#include "feniamanager.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commandtemplate.h"
#include "hometown.h"
#include "occupations.h"
#include "core/behavior/behavior_utils.h"
#include "../loadsave/behavior_utils.h"
#include "areaquestutils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "msgformatter.h"
#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"


Trainer::Trainer( )
{
}

int Trainer::getOccupation( )
{
    return BasicMobileDestiny::getOccupation( ) | (1 << OCC_TRAINER);
}



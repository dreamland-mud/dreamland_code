/* $Id: class_antipaladin.cpp,v 1.1.2.21.4.14 2009/09/11 11:24:54 rufina Exp $
 * 
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include "logstream.h"

#include "spelltemplate.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "core/behavior/behavior_utils.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"


#include "magic.h"
#include "fight.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "weapongenerator.h"
#include "vnum.h"
#include "fenia_utils.h"
#include "tohit.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "loadsave.h"
#include "def.h"
#include "skill_utils.h"
#include "l10n.h"

PROF(anti_paladin);

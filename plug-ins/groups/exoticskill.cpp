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
#include "exoticskill.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"

#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"

/*
 * ExoticSkill
 */
bool ExoticSkill::visible( CharacterMemoryInterface * ) const
{
    return false;
}
bool ExoticSkill::available( Character * ) const
{
    return false;
}
bool ExoticSkill::usable( Character *, bool ) const
{
    return false;
}
int ExoticSkill::getLearned( Character *ch ) const
{
    return min( 100, ch->getRealLevel( ) * max(1, (3 + (ch->getCurrStat(STAT_INT) - 20))) );
}




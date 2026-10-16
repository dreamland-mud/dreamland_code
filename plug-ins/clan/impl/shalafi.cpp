/* $Id: shalafi.cpp,v 1.1.6.7.6.12 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
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

#include "clanmobiles.h"
#include "clanrecords.h"

#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "damage.h"
#include "fight.h"
#include "magic.h"
#include "merc.h"


#include "vnum.h"
#include "loadsave.h"
#include "save.h"
#include "interp.h"
#include "act.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "def.h"
#include "l10n.h"

RELIG(alala);
RELIG(ares);
RELIG(turlok);
RELIG(goktengri);
GSN(acid_arrow);
GSN(acid_blast);
GSN(blindness);
GSN(caustic_font);
GSN(dispel_affects);
GSN(energy_drain);
GSN(lightning_breath);
GSN(plague);
GSN(poison);
GSN(spellbane);
GSN(weaken);

/*--------------------------------------------------------------------------
 * Seneschal 
 *-------------------------------------------------------------------------*/

SPELL_DECL(Transform);
VOID_SPELL(Transform)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;
  int hp_modif;

  if (ch->isAffected(sn) || ch->hit > ch->max_hit)
    {
      oldact(_("Ты уже переполне$gно|н|на жизненной энергией."), ch, 0, 0, TO_CHAR);
      return;
    }

  hp_modif = ( min(30000 - ch->max_hit, ( int )( ch->max_hit / 1.6 ) ) );
  ch->hit += hp_modif;

  af.type               = sn;
  af.level              = level;
  af.duration           = 24;

  af.location = APPLY_HIT;
  af.modifier           = hp_modif;
  affect_to_char(ch,&af);

  af.location = APPLY_DEX;
  af.modifier  = - (4 + level / 10);
  af.bitvector.setTable(&affect_flags);
  af.bitvector.setValue(AFF_SLOW);    
  affect_to_char( ch, &af );

  ch->pecho(_("Прилив жизненной силы затмевает твой разум."));

}



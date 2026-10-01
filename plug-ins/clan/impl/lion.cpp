/* $Id: lion.cpp,v 1.1.6.9.6.17 2010-09-01 21:20:44 rufina Exp $
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

#include "objectbehavior.h"
#include "clanmobiles.h"

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "race.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "act.h"
#include "interp.h"

#include "merc.h"
#include "move_utils.h"
#include "vnum.h"
#include "clanownshook.h"
#include "clanreference.h"

#include "loadsave.h"
#include "fight.h"
#include "magic.h"
#include "def.h"
#include "skill_utils.h"
#include "l10n.h"

CLAN(artificer);
GSN(acid_arrow);
GSN(acid_blast);
GSN(caustic_font);
GSN(claw);
GSN(dispel_affects);
GSN(spellbane);

#define OBJ_VNUM_EYED_SWORD                503        
#define OBJ_VNUM_LION_SHIELD                33

/*--------------------------------------------------------------------------
 * LionMan 
 *-------------------------------------------------------------------------*/



SPELL_DECL(EvolveLion);
VOID_SPELL(EvolveLion)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->is_npc())
      return;

  if ( ch->isAffected(sn ) )
        {
                ch->pecho(_("Ты уже трансформирова{Smлся{Sfлась{Sx во льва."));
                return;
        }

  ch->hit += ch->getPC()->perm_hit / 2;

  af.type      = sn;
  af.level     = level; 
  af.duration  = 3 + level / 30;
  af.location = APPLY_HIT;
  af.modifier  = ch->getPC()->perm_hit / 2;
  affect_to_char(ch,&af);

  af.bitvector.setTable(&affect_flags);
  af.type      = sn;
  af.level     = level;
  af.duration  = 3 + level / 30;
  af.location = APPLY_DEX;
  af.modifier  = -(1 + level / 20);
  af.bitvector.setValue(AFF_SLOW);
  affect_to_char( ch, &af );

  af.bitvector.setTable(&affect_flags);
  af.type      = sn;
  af.level     = level;
  af.duration  = 3 + level / 30;
  af.location = APPLY_DAMROLL;
  af.modifier  = clan_char_owns( ch, "evolve-lion+" ) ? level * 5 / 8 : level / 2;
  af.bitvector.setValue(AFF_BERSERK);
  affect_to_char( ch, &af );

  oldact_p(_("Ты чувствуешь себя немного неповоротлив$gым|ым|ой, но зато намного более сильн$gым|ым|ой."),
                ch,0,0,TO_CHAR,POS_RESTING);
  oldact(_("Кожа $c2 становится серой!"),ch,0,0,TO_ROOM);

}




SPELL_DECL(EyesOfTiger);
VOID_SPELL(EyesOfTiger)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if (DIGGED(victim))
        {
                ch->pecho(_("Твой львиный глаз не может найти такого."));
                return;
        }

        if (victim->is_npc() || victim->getPC()->getClan() != clan_artificer)
        {
                ch->pecho(_("Ты можешь следить только за мастерами Артели!"));
                return;
        }
        
        if (is_safe_nomessage(ch,victim)) 
        {
                ch->pecho(_("Твой львиный глаз не смог найти такого."));
                return;
        }
        
        Room *was_in = ch->in_room;
        ch->in_room = victim->in_room;
        interpret_raw(ch, "look", "auto");
        ch->in_room = was_in;
}




SPELL_DECL(Prevent);
VOID_SPELL(Prevent)::run( Character *ch, Character *victim, int sn, int level ) 
{
    Affect af;

    if (ch->isAffected( sn )) {
        oldact(_("Ты уже защище$gно|н|на от мин Артели."), ch, 0, 0, TO_CHAR);
        return;
    }

    af.type               = sn;
    af.level              = level; 
    af.duration           = max( 6, ch->getPC( )->getClanLevel( ) * 2 );
    
    affect_to_char(ch, &af);  

    ch->pecho( _("Ты защищаешь себя от мин Артели.") );
}

VOID_SPELL(Prevent)::run( Character *ch, Room *room, int sn, int level ) 
{ 
        Affect af;

        if ( room->isAffected( sn ))
        {
                ch->pecho(_("Это место уже защищено от мин и пневмопочты Артели."));
                return;
        }

        af.bitvector.setTable(&raffect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = max( 1, ch->getPC( )->getClanLevel( ) * 1 );
        
        af.modifier  = 0;
        af.bitvector.setValue(AFF_ROOM_PREVENT);
        room->affectTo( &af );

        ch->pecho(_("Ты защищаешь местность от мин и пневмопочты Артели."));
        oldact(_("$c1 защищает местность от мин и пневмопочты Артели."),ch,0,0,TO_ROOM);
}

AFFECT_DECL(Prevent);
VOID_AFFECT(Prevent)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, _("Местность на {W%1$d{x ча%1$Iс|са|сов защищена от мин и пневмопочты Артели."), paf->duration )
        << endl;
}



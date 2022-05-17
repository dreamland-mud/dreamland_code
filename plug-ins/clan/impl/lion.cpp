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

#include "lion.h"

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
#include "clanreference.h"
#include "mercdb.h"
#include "handler.h"
#include "fight.h"
#include "magic.h"
#include "def.h"
#include "skill_utils.h"

CLAN(hunter);
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
void ClanGuardLion::actGreet( PCharacter *wch )
{
    do_say(ch, "Добро пожаловать, странник.");
}
void ClanGuardLion::actPush( PCharacter *wch )
{
    oldact("$C1 выпускает когти.\n\rИ ты быстренько убираешься из этой местности.", wch, 0, ch, TO_CHAR );
    oldact("$C1, глядя на $c4, выпускает когти, и $c1 сматывает удочки.", wch, 0, ch, TO_ROOM );
}
int ClanGuardLion::getCast( Character *victim )
{
        int sn = -1;

        switch ( dice(1,16) )
        {
        case  0: 
        case  1:
                if (!victim->isAffected( gsn_spellbane ))
                    sn = gsn_dispel_affects;
                break;
        case  2:
        case  3:
                sn = gsn_acid_blast;
                break;
        case  4: 
        case  5:
                sn = gsn_caustic_font;
                break; 
        case  6:
        case  7:
        case  8:
                sn = gsn_acid_arrow;
                break;
        default:
                sn = -1;
                break;
        }

        return sn;
}

bool ClanGuardLion::specFight( )
{
        Character *victim;

        if (!( victim = getVictim( ) ))
            return true;

        if ( number_percent() < 33 )
        {
                int damage_claw;

                damage_claw = dice(ch->getRealLevel(), 24) + ch->damroll;
                damage(ch, victim, damage_claw, gsn_claw, DAM_BASH, true, DAMF_WEAPON);
                return true;
        }

        spec_cast( victim );
        return true;
}


SPELL_DECL(EvolveLion);
VOID_SPELL(EvolveLion)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->is_npc())
      return;

  if ( ch->isAffected(sn ) )
        {
                ch->pecho("Ты уже трансформирова{Smлся{Sfлась{Sx во льва.");
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
  af.modifier  = level / 2;
  af.bitvector.setValue(AFF_BERSERK);
  affect_to_char( ch, &af );

  oldact_p("Ты чувствуешь себя немного неповоротлив$gым|ым|ой, но зато намного более сильн$gым|ым|ой.",
                ch,0,0,TO_CHAR,POS_RESTING);
  oldact("Кожа $c2 становится серой!",ch,0,0,TO_ROOM);

}




SPELL_DECL(EyesOfTiger);
VOID_SPELL(EyesOfTiger)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if (DIGGED(victim))
        {
                ch->pecho("Твой львиный глаз не может найти такого.");
                return;
        }

        if (victim->is_npc() || victim->getPC()->getClan() != clan_hunter)
        {
                ch->pecho("Ты можешь следить только за Охотниками!");
                return;
        }
        
        if (is_safe_nomessage(ch,victim)) 
        {
                ch->pecho("Твой львиный глаз не смог найти такого.");
                return;
        }
        
        do_look_auto( ch, victim->in_room );
}




SPELL_DECL(Prevent);
VOID_SPELL(Prevent)::run( Character *ch, Character *victim, int sn, int level ) 
{
    Affect af;

    if (ch->isAffected( sn )) {
        oldact("Ты уже защище$gно|н|на от ловушек Охотников.", ch, 0, 0, TO_CHAR);
        return;
    }

    af.type               = sn;
    af.level              = level; 
    af.duration           = max( 6, ch->getPC( )->getClanLevel( ) * 2 );
    
    affect_to_char(ch, &af);  

    ch->pecho( "Ты защищаешь себя от ловушек Охотников." );
}

VOID_SPELL(Prevent)::run( Character *ch, Room *room, int sn, int level ) 
{ 
        Affect af;

        if ( room->isAffected( sn ))
        {
                ch->pecho("Это место уже защищено от мести и ловушек Охотников.");
                return;
        }

        af.bitvector.setTable(&raffect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = max( 1, ch->getPC( )->getClanLevel( ) * 1 );
        
        af.modifier  = 0;
        af.bitvector.setValue(AFF_ROOM_PREVENT);
        room->affectTo( &af );

        ch->pecho("Ты защищаешь местность от ловушек Охотников и от их мести.");
        oldact("$c1 защищает местность от ловушек Охотников и от их мести.",ch,0,0,TO_ROOM);
}

AFFECT_DECL(Prevent);
VOID_AFFECT(Prevent)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Местность на {W%1$d{x ча%1$Iс|са|сов защищена от ловушек и мести Охотников.", paf->duration )
        << endl;
}



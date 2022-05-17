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

#include "shalafi.h"

#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"
#include "religion.h"

#include "damage.h"
#include "fight.h"
#include "magic.h"
#include "merc.h"

#include "mercdb.h"
#include "vnum.h"
#include "handler.h"
#include "save.h"
#include "interp.h"
#include "act.h"
#include "act_move.h"
#include "def.h"

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
GSN(mental_attack);
GSN(mental_knife);
GSN(plague);
GSN(poison);
GSN(spellbane);
GSN(weaken);

/*--------------------------------------------------------------------------
 * Seneschal 
 *-------------------------------------------------------------------------*/
void ClanGuardShalafi::actGreet( PCharacter *wch )
{
    do_say(ch, "Приветствую тебя, мудрец.");
}
void ClanGuardShalafi::actPush( PCharacter *wch )
{
    oldact("$C1 бросает на тебя мимолетный взгляд.\n\rИ тут же ты чувствуешь, как некая магическая сила вышвыривает тебя вон.", wch, 0, ch, TO_CHAR );
    oldact("$C1 бросает на $c4 мимолетный взгляд и $c1 мгновенно исчезает.", wch, 0, ch, TO_ROOM );
}
int ClanGuardShalafi::getCast( Character *victim )
{
        int sn = -1;

        switch ( dice(1,16) )
        {
        case  0:
                sn = gsn_blindness;
                break;
        case  1:
                if (!victim->isAffected( gsn_spellbane ))
                    sn = gsn_dispel_affects;
                break;
        case  2:
                sn = gsn_weaken;
                break;
        case  3:
                sn = gsn_blindness;
                break;
        case  4:
                sn = gsn_acid_arrow;
                break;
        case  5:
                sn = gsn_caustic_font;
                break;
        case  6:
                sn = gsn_energy_drain;
                break;
        case  7:
        case  8:
        case  9:
                sn = gsn_acid_blast;
                break;
        case 10:
                sn = gsn_plague;
                break;
        case 11:
                sn = gsn_acid_blast;
                break;
        case 12:  
        case 13:
                sn = gsn_lightning_breath;
                break;
        case 14:
        case 15:
                sn = gsn_mental_knife;
                break;
        default:
                sn = -1;
                break;
        }

        return sn;
}


ShalafiFaculty::ShalafiFaculty() 
     : classes(professionManager)
{    
}

bool ShalafiFaculty::canInduct(PCMemoryInterface *pci) const
{
    return classes.isSet(pci->getProfession());
}

bool ShalafiClan::canInduct(PCharacter *ch) const
{
    if (!DefaultClan::canInduct(ch))
        return false;

    // TODO these checks can be changed to "ch.religion.path != 'rage'", 
    // once a Path becomes a separate concept in the code.
    if (ch->getReligion() == god_alala 
        || ch->getReligion() == god_ares
        || ch->getReligion() == god_turlok
        || ch->getReligion() == god_goktengri)
        return false;

    if (ch->getCurrStat(STAT_INT) <= 23)
        return false;
        
    return true;
}

void ShalafiClan::onInduct(PCharacter *ch) const
{
    const DLString &prof = ch->getProfession()->getName();

    for (auto &o: *getOrgs())
        if (o.second->canInduct(ch)) {
            ClanOrgs::setAttr(ch, o.first);
            ch->pecho("Ты поступаешь на {b%N4{x.", o.second->shortDescr.c_str());
            return;
        }

    LogStream::sendWarning() << "Shalafi: no faculty found for class "  << prof << endl;
}




SPELL_DECL(MentalKnife);
VOID_SPELL(MentalKnife)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;
  int dam;

  if( ch->getModifyLevel() < 20 )
    dam = dice( level, 7 );
  else if( ch->getModifyLevel() < 40 )
    dam = dice( level, 8 );
  else if( ch->getModifyLevel() < 65)
    dam = dice( level, 11 );
  else if( ch->getModifyLevel() < 85)
    dam = dice( level, 15 );
  else if( ch->getModifyLevel() < 95)
    dam = dice( level, 18 );
  else dam = dice( level, 20 );

  if (saves_spell(level,victim, DAM_MENTAL, ch, DAMF_MAGIC))
              dam /= 2;
  if( victim->is_npc() ) dam /= 4;

  ch->setWait(gsn_mental_attack->getBeats(ch) );

  try {
      damage_nocatch( ch, victim, ( dam ), sn, DAM_MENTAL, true, DAMF_MAGIC );
        
      if(!victim->isAffected(sn) && !saves_spell(level, victim, DAM_MENTAL, ch, DAMF_MAGIC))
        {
          af.type               = sn;
          af.level              = level;
          af.duration           = level;

          af.modifier           = ( -7 );
          af.location = APPLY_INT;
          affect_to_char(victim,&af);

          af.location = APPLY_WIS;
          affect_to_char(victim,&af);

          if (ch != victim) {
            oldact("Твой ментальный удар повреждает разум $C2!", ch,0,victim,TO_CHAR);
            oldact("Ментальный удар $c2 повреждает твой разум!", ch,0,victim,TO_VICT);
            oldact("Ментальный удар $c2 повреждает разум $C2!", ch,0,victim,TO_NOTVICT);
          }
          else {
            oldact("Ментальный удар повреждает твой разум!", ch,0,0,TO_CHAR);
            oldact("Ментальный удар $c2 повреждает $s разум!", ch,0,0,TO_ROOM);
          }
        }
  } catch (const VictimDeathException &) {
  }

}

TYPE_SPELL(bool, MentalKnife)::spellbane( Character *, Character * ) const
{
    return false;
}



SPELL_DECL(Scourge);
VOID_SPELL(Scourge)::run( Character *ch, Room *room, int sn, int level ) 
{ 

  int dam;

  if( ch->getModifyLevel() < 40 )
        dam = dice(level,6);
  else if( ch->getModifyLevel() < 65)
        dam = dice(level,9);
  else dam = dice(level,12);


        for(auto &tmp_vict : ch->in_room->getPeople())
        {
            if(!tmp_vict->isDead() && tmp_vict->in_room == ch->in_room){

        if ( tmp_vict->is_mirror()
            && ( number_percent() < 50 ) ) continue;
                        

      if ( !is_safe_spell(ch,tmp_vict,true))
        {
            if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                yell_panic(ch, tmp_vict, "Помогите! Кто-то напал на меня!", "Помогите! %1$^C1 напа%1$Gло|л|ла на меня!");

        
          if (!tmp_vict->isAffected(sn)) {
        
          try{
            if (number_percent() < level)
              spell(gsn_poison, level, ch, tmp_vict);

            if (number_percent() < level)
              spell(gsn_blindness,level,ch,tmp_vict);

            if (number_percent() < level)
              spell(gsn_weaken, level, ch, tmp_vict);

            if (saves_spell(level,tmp_vict, DAM_FIRE, ch, DAMF_MAGIC))
              dam /= 2;
            damage_nocatch( ch, tmp_vict, ( dam ), sn, DAM_FIRE, true, DAMF_MAGIC );
          }
            catch (const VictimDeathException &) {
                   continue;
            }
          }
        }

      }
    }
}


SPELL_DECL(Transform);
VOID_SPELL(Transform)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;
  int hp_modif;

  if (ch->isAffected(sn) || ch->hit > ch->max_hit)
    {
      oldact("Ты уже переполне$gно|н|на жизненной энергией.", ch, 0, 0, TO_CHAR);
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

  ch->pecho("Прилив жизненной силы затмевает твой разум.");

}




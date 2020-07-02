
/* $Id: group_combat.cpp,v 1.1.2.17.6.11 2009/09/11 11:24:54 rufina Exp $
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

#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "handler.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "effects.h"
#include "def.h"



SPELL_DECL(ChainLightning);
VOID_SPELL(ChainLightning)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Character *last_vict;
    bool found;
    int dam;
    list<Character*> people = ch->in_room->getPeople();

    /* first strike */

    act_p("Разряд молнии, созданный $c5, поражает $C4.",
           ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("Созданный тобой разряд молнии поражает $C4.",
           ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("Разряд молнии, созданный $c5, поражает тебя!",
           ch,0,victim,TO_VICT,POS_RESTING);
  
    // special damage table   
         if (level <= 20)   dam = dice( level, 4 );    
    else if (level <= 40)   dam = dice( level, 5 );
    else if (level <= 70)   dam = dice( level, 6 );
    else                    dam = dice( level, 7 );
    
    if (saves_spell(level,victim,DAM_LIGHTNING,ch, DAMF_SPELL)){
        dam /= 2;
    }

    try {
        damage_nocatch(ch,victim,dam,sn,DAM_LIGHTNING,true, DAMF_SPELL);
    }
    catch (const VictimDeathException &){
        people.remove(victim);
    }
    
    last_vict = victim;
    level -= 4;   /* decrement damage */

    /* new targets */
    while (level > 0)
    {
        found = false;        

        for (auto &tmp_vict : people)
        {
          
          if (!is_safe_spell(ch,tmp_vict,true) && tmp_vict != last_vict)
          {

            if ( tmp_vict->is_mirror()
            && ( number_percent() < 50 ) ) continue;

            found = true;
            last_vict = tmp_vict;
            if (is_safe(ch, tmp_vict) )  {
              act_p( "Разряд молнии затухает, не достигнув $c2.",
                      ch, 0, 0, TO_ROOM,POS_RESTING);
              act_p( "Разряд молнии затухает, не достигнув тебя.",
                      ch, 0, 0, TO_CHAR,POS_RESTING);
            }
            else  {
              act_p("Разряд молнии поражает $c4!",
                     tmp_vict,0,0,TO_ROOM,POS_RESTING);
              act_p("Разряд молнии поражает тебя!",
                     tmp_vict,0,0,TO_CHAR,POS_RESTING);
                
                // special damage table   
                     if (level <= 20)   dam = dice( level, 4 );    
                else if (level <= 40)   dam = dice( level, 5 );
                else if (level <= 70)   dam = dice( level, 6 );
                else                    dam = dice( level, 7 );
                
                if (ch->fighting != tmp_vict && tmp_vict->fighting != ch)
                    yell_panic( ch, tmp_vict );

              if (saves_spell(level,tmp_vict,DAM_LIGHTNING,ch, DAMF_SPELL)){
                dam /= 2;
              }

                try{
              damage_nocatch(ch,tmp_vict,dam,sn,DAM_LIGHTNING,true, DAMF_SPELL);
              shock_effect(tmp_vict,level,dam,TARGET_CHAR, DAMF_SPELL);                    
                }
                catch (const VictimDeathException &){
                    level -= 4;
                    people.remove(tmp_vict);
                    break;
                }
              level -= 4;  /* decrement damage */
            }
          }
        }   /* end target searching loop */

        if (!found) /* no target found, hit the caster */
        {
          if (ch == 0 )
            return;

          if (last_vict == ch) /* no double hits */
          {
            act_p("Разряд молнии исчезает.",ch,0,0,TO_ROOM,POS_RESTING);
            act_p("Разряд молнии исчезает, не достигнув тебя.",
                   ch,0,0,TO_CHAR,POS_RESTING);
            return;
          }

          last_vict = ch;
          act_p("Разряд молнии поражает $c4..!",ch,0,0,TO_ROOM,POS_RESTING);
          ch->send_to("Созданная тобой молния поражает тебя же!\n\r");

            // special damage table   
            if (level <= 20)   dam = dice( level, 4 );    
            else if (level <= 40)   dam = dice( level, 5 );
            else if (level <= 70)   dam = dice( level, 6 );
            else                    dam = dice( level, 7 );  
            
          if (saves_spell(level,ch,DAM_LIGHTNING,ch, DAMF_SPELL))
           dam /= 2;
          damage(ch,ch,dam,sn,DAM_LIGHTNING,true, DAMF_SPELL);
          level -= 4;  /* decrement damage */
          if (ch == 0)
            return;
        }
    /* now go back and find more targets */
    }

}

SPELL_DECL(ChillTouch);
VOID_SPELL(ChillTouch)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;
    
    act_p("Прикосновение ледяной длани $c2 замораживает $C4!",
           ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("Прикосновение твоей ледяной длани замораживает $C4!",
           ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("Прикосновение ледяной длани $c2 замораживает тебя!",
           ch,0,victim,TO_VICT,POS_RESTING);

    // Tier 2 damage (spell has regular effects)
         if (level <= 20)   dam = dice( level, 8  );    
    else if (level <= 40)   dam = dice( level, 12 );
    else if (level <= 70)   dam = dice( level, 15 );
    else                    dam = dice( level, 18 );
    
    if ( saves_spell( level, victim,DAM_COLD,ch, DAMF_SPELL ) )
        dam /= 2;

    damage_nocatch( ch, victim, dam, sn, DAM_COLD,true, DAMF_SPELL );
    cold_effect(victim,level,dam,TARGET_CHAR, DAMF_SPELL);    
}


SPELL_DECL(ColourSpray);
VOID_SPELL(ColourSpray)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    act_p("Созданная $c5 {Rр{Yа{Gд{Cу{Bг{Mа{x внезапно выстреливает в сторону $C4 разноцветными лучами!",
           ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("Созданная тобой {Rр{Yа{Gд{Cу{Bг{Mа{x внезапно выстреливает в сторону $C4 разноцветными лучами!",
           ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("Созданная $c5 {Rр{Yа{Gд{Cу{Bг{Mа{x внезапно выстреливает в тебя разноцветными лучами!",
           ch,0,victim,TO_VICT,POS_RESTING);

    // Tier 3 damage: spells with powerful effects
         if (level <= 20)   dam = dice( level, 7  );    
    else if (level <= 40)   dam = dice( level, 10 );
    else if (level <= 70)   dam = dice( level, 13 );
    else                    dam = dice( level, 16 );
    
    if ( saves_spell( level, victim,DAM_LIGHT,ch, DAMF_SPELL) )
        dam /= 2;
    else
        spell(gsn_blindness, level, ch, victim );

    damage_nocatch( ch, victim, dam, sn, DAM_LIGHT,true, DAMF_SPELL );
}


SPELL_DECL(DesertFist);
VOID_SPELL(DesertFist)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        
    int dam;

    if (   (ch->in_room->sector_type != SECT_HILLS)
        && (ch->in_room->sector_type != SECT_MOUNTAIN)
        && (ch->in_room->sector_type != SECT_DESERT) )
    {
        ch->println("Здесь недостаточно песка, чтобы сформировать кулак.");
        ch->wait = 0;
        return;
    }

    act_p("{yВихрь песка,{x созданный $c5, образует огромный кулак и ударяет $c4.",
        ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("{yВихрь песка,{x созданный тобой, образует огромный кулак и ударяет $c4.",
        ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("{yВихрь песка,{x созданный $c5, образует огромный кулак и ударяет тебя!",
        ch,0,victim,TO_VICT,POS_RESTING);

    // Tier 1 damage: only spells with severe limitations
         if (level <= 20)   dam = dice( level, 10 );    
    else if (level <= 40)   dam = dice( level, 13 );
    else if (level <= 70)   dam = dice( level, 16 );
    else                    dam = dice( level, 20 );

    damage_nocatch(ch,victim,dam,sn,DAM_BASH,true, DAMF_SPELL);
    sand_effect(victim,level,dam,TARGET_CHAR, DAMF_SPELL);
}

SPELL_DECL(EtheralFist);
VOID_SPELL(EtheralFist)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    act_p("Призванный $C5 кулак из иномирового эфира сокрушает $C4!",
           ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("Призванный тобой кулак из иномирового эфира сокрушает $C4!",
           ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("Призванный $C5 кулак из иномирового эфира сокрушает тебя!",
           ch,0,victim,TO_VICT,POS_RESTING);
    
    // Tier 3 damage: spells with powerful effects
         if (level <= 20)   dam = dice( level, 7  );    
    else if (level <= 40)   dam = dice( level, 10 );
    else if (level <= 70)   dam = dice( level, 13 );
    else                    dam = dice( level, 16 );
        
    if ( saves_spell( level, victim,DAM_ENERGY,ch, DAMF_SPELL) )
        dam /= 2;
    else {
        if ( number_percent() > 50 ) {
            act_p("$C4 ошеломленно трясет головой!", 0,0,victim,TO_CHAR,POS_RESTING);
            act_p("$C4 ошеломленно трясет головой!", 0,0,victim,TO_NONVICT,POS_RESTING);
            victim->println("Ты ошеломленно трясешь головой и не можешь пошевелиться!");            
            victim->setWaitViolence( 2 );
        }    
    }    

    damage_nocatch( ch, victim, dam, sn,DAM_ENERGY,true, DAMF_SPELL);
}

SPELL_DECL(HandOfUndead);
VOID_SPELL(HandOfUndead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    if ( (victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD)) ||
         IS_VAMPIRE(victim) )
        {
            ch->println("Рука умертвия не властна над твоей жертвой.");
            return;
        }    

    act_p("Выпущенная $C5 рука умертвия пытается схватить $C4!",
           ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("Выпущенная тобой рука умертвия пытается схватить $C4!",
           ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("Выпущенная $C5 рука умертвия пытается схватить тебя!",
           ch,0,victim,TO_VICT,POS_RESTING);
   
    // Tier 2 damage: spells with regular or no special effects   
         if (level <= 20)   dam = dice( level, 8  );    
    else if (level <= 40)   dam = dice( level, 12 );
    else if (level <= 70)   dam = dice( level, 15 );
    else                    dam = dice( level, 18 );

    // instakills small mobs
    if ( victim->getModifyLevel() < ( level/10 ) )
    {
                            dam = victim->hit + 1;
    }
    
    if ( saves_spell( level, victim,DAM_NEGATIVE,ch, DAMF_SPELL) )
    {         
        act("Ты на мгновение почувствова$gло|л|ла озноб.", victim, 0, 0, TO_CHAR);            
        return;  
    }

    victim->mana        /= 2;
    victim->move        /= 2;
    ch->hit             += dam / 2;

    act("Призрачные когти смыкаются вокруг $C4!", 0,0,victim,TO_NOTVICT);    
    victim->println("Ты чувствуешь, как жизнь ускользает от тебя!");
    ch->println("Ты чувствуешь прилив жизненной энергии!");    

    damage_nocatch( ch, victim, dam, sn,DAM_NEGATIVE,true, DAMF_SPELL);
}


SPELL_DECL(Iceball);
VOID_SPELL(Iceball)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    int dam;
    int movedam;

    // Tier 1 damage: only spells with severe limitations
         if (level <= 20)   dam = dice( level, 10 );    
    else if (level <= 40)   dam = dice( level, 13 );
    else if (level <= 70)   dam = dice( level, 16 );
    else                    dam = dice( level, 20 );

    movedam     = number_range( ch->getModifyLevel(), 2 * ch->getModifyLevel() );
     
        for(auto &it : ch->in_room->getPeople())
        {
            if(!it->isDead() && it->in_room == ch->in_room){

                if ( it->is_mirror()
                    && ( number_percent() < 50 ) ) continue;

                if ( !is_safe_spell(ch,it,true))
                {
                        if (ch->fighting != it && it->fighting != ch)
                            yell_panic( ch, it );

                        if (saves_spell(level,it, DAM_COLD,ch, DAMF_SPELL))
                                dam /= 2;
                    try{
                        damage_nocatch( ch, it, dam, sn, DAM_COLD, true, DAMF_SPELL );
                        cold_effect(it,level,dam,TARGET_CHAR, DAMF_SPELL);                        
                        it->move -= min((int)it->move,movedam);
                     }
                         catch (const VictimDeathException &) {
                             continue;
                    }
                }                       
             }
        }
 }


SPELL_DECL(MagicMissile);
VOID_SPELL(MagicMissile)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int mlevel;    

    static const int dam_each[] =
    {
         0,
         3,  3,  4,  4,  5,         6,  6,  6,  6,  6,
         7,  7,  7,  7,  7,         8,  8,  8,  8,  8,
         9,  9,  9,  9,  9,        10, 10, 10, 10, 10,
        11, 11, 11, 11, 11,        12, 12, 12, 12, 12,
        13, 13, 13, 13, 13,        14, 14, 14, 14, 14
    };

    int dam;

    if (victim->isAffected(gsn_shield ))  {
        act("Твоя волшебная стрела исчезает, не достигнув цели.", ch, 0, victim, TO_CHAR);

        if (victim != ch)
            act("Твой щит блокирует волшебную стрелу $c2.", ch, 0, victim, TO_VICT);

        return;
    }

    mlevel = ch->getModifyLevel( );
    level = min(level, (int)( sizeof(dam_each)/sizeof(dam_each[0]) - 1 ) );
    level = max( 0, level);
    
    if( mlevel > 50)
        dam = level / 4;
    else
        dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
    
    if ( saves_spell( level, victim,DAM_ENERGY,ch, DAMF_SPELL) )
        dam /= 2;
    damage_nocatch( ch, victim, dam, sn, DAM_ENERGY ,true, DAMF_SPELL);

    if ( mlevel > 4 )  {
      dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
      if ( saves_spell( level, victim,DAM_ENERGY,ch, DAMF_SPELL) )
          dam /= 2;
      damage_nocatch( ch, victim, dam, sn, DAM_ENERGY ,true, DAMF_SPELL);
    }
    if ( mlevel > 8 )  {
      dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
      if ( saves_spell( level, victim,DAM_ENERGY,ch, DAMF_SPELL) )
          dam /= 2;
      damage_nocatch( ch, victim, dam, sn, DAM_ENERGY ,true, DAMF_SPELL);
    }
    if( mlevel > 12 )  {
      dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
      if ( saves_spell( level, victim,DAM_ENERGY,ch, DAMF_SPELL) )
          dam /= 2;
      damage_nocatch( ch, victim, dam, sn, DAM_ENERGY ,true, DAMF_SPELL);
    }
    if( mlevel > 16 )  {
      dam = number_range( dam_each[level] / 2, dam_each[level] * 2 );
      if ( saves_spell( level, victim,DAM_ENERGY,ch, DAMF_SPELL) )
          dam /= 2;
      damage_nocatch( ch, victim, dam, sn, DAM_ENERGY ,true, DAMF_SPELL);
    }
}


SPELL_DECL(SandStorm);
VOID_SPELL(SandStorm)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    int dam;

    if ( ch->in_room->sector_type == SECT_AIR || 
         ch->in_room->sector_type == SECT_INSIDE || 
         ch->in_room->sector_type == SECT_WATER_SWIM || 
         ch->in_room->sector_type == SECT_WATER_NOSWIM )
    {
        ch->send_to("Здесь нет ни крупицы песка!\n\r");
        ch->wait = 0;
        return;
    }

    // Tier 1 damage: only spells with severe limitations
         if (level <= 20)   dam = dice( level, 10 );    
    else if (level <= 40)   dam = dice( level, 13 );
    else if (level <= 70)   dam = dice( level, 16 );
    else                    dam = dice( level, 20 );

    if ( ch->in_room->sector_type == SECT_DESERT) {
        act_p("$c1 создает мощную пустынную бурю вокруг себя!",ch,0,0,TO_ROOM,POS_RESTING);
        ch->send_to("Ты создаешь мощную пустынную бурю вокруг себя.\n\r");        
        dam = dice( level, 24 );        
    }
    else {
        act_p("$c1 создает пылевой вихрь вокруг себя.",ch,0,0,TO_ROOM,POS_RESTING);
        ch->send_to("Ты создаешь пылевой вихрь вокруг себя.\n\r");
    }
    sand_effect(room,level,dam/2,TARGET_ROOM, DAMF_SPELL);

        for ( auto &vch : room->getPeople() )
        {
            if(!vch->isDead() && vch->in_room == room){

                if (vch->is_mirror() && number_percent() < 50) 
                continue;

               if ( is_safe_spell(ch,vch,true )
                        || ( vch->is_npc()
                                && ch->is_npc()
                                && ( ch->fighting != vch && vch->fighting != ch)))
                        continue;
                if ( is_safe(ch, vch) )
                        continue;

                if (ch->fighting != vch && vch->fighting != ch)
                yell_panic( ch, vch );

            try{
                if ( saves_spell(level,vch,DAM_COLD,ch, DAMF_SPELL) )
                {
                        sand_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                        damage_nocatch(ch,vch,dam/2,sn,DAM_OTHER,true, DAMF_SPELL);
                }
                else
                {
                        sand_effect(vch,level,dam,TARGET_CHAR, DAMF_SPELL);
                        damage_nocatch(ch,vch,dam,sn,DAM_OTHER,true, DAMF_SPELL);
                }
            }
            catch (const VictimDeathException &){
                continue;
            }
        }
        }
}

SPELL_DECL(ShockingGrasp);
VOID_SPELL(ShockingGrasp)::run( Character *ch, Character *victim, int sn, int level ) 
{ 

    int dam;
    
    // Tier 2 damage: spells with regular or no special effects   
         if (level <= 20)   dam = dice( level, 8  );    
    else if (level <= 40)   dam = dice( level, 12 );
    else if (level <= 70)   dam = dice( level, 15 );
    else                    dam = dice( level, 18 );
    
    if ( saves_spell( level, victim,DAM_LIGHTNING,ch, DAMF_SPELL) )
        dam /= 2;
    
    damage_nocatch( ch, victim, dam, sn, DAM_LIGHTNING ,true, DAMF_SPELL);
    shock_effect(victim,level,dam,TARGET_CHAR, DAMF_SPELL);    
}

SPELL_DECL(VampiricBlast);
VOID_SPELL(VampiricBlast)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam, chance;    
    Affect af;

    chance = 50 + ch->getCurrStat(STAT_INT) - victim->getCurrStat(STAT_INT);
 
    // Tier 2 damage: spells with regular or no special effects   
         if (level <= 20)   dam = dice( level, 8  );    
    else if (level <= 40)   dam = dice( level, 12 );
    else if (level <= 70)   dam = dice( level, 15 );
    else                    dam = dice( level, 18 );
    
    if ( saves_spell( level, victim, DAM_NEGATIVE,ch, DAMF_SPELL ) ) {
        dam /= 2;
    }
    else {
        if ( (number_percent() < chance) && (!victim->isAffected(gsn_weaken)) ) {
            af.where     = TO_AFFECTS;
            af.type      = gsn_weaken;
            af.level     = level;
            af.duration  = (4 + level / 12);
            af.location  = APPLY_STR;
            af.modifier  = -1 * (2 + level / 12);
            af.bitvector = AFF_WEAKEN;
            affect_to_char( victim, &af );
            victim->send_to("Ты чувствуешь, как {Dтемная магия{x отнимает у тебя последние силы!\n\r");
            act_p("$c1 слабеет на глазах.",victim,0,0,TO_ROOM,POS_RESTING);            
        } 
    }    
    damage_nocatch( ch, victim, dam, sn,DAM_NEGATIVE,true, DAMF_SPELL);
}

SPELL_DECL(Hurricane);
VOID_SPELL(Hurricane)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    int dam;

    act_p("$c1 призывает повелителя ураганов на помощь.",
           ch,0,0,TO_NOTVICT,POS_RESTING);
    act_p("Ты призываешь повелителя ураганов на помощь.",
           ch,0,0,TO_CHAR,POS_RESTING);

    // Tier 2 damage: spells with regular or no special effects   
         if (level <= 20)   dam = dice( level, 8  );    
    else if (level <= 40)   dam = dice( level, 12 );
    else if (level <= 70)   dam = dice( level, 15 );
    else                    dam = dice( level, 18 );

    for ( auto &vch : room->getPeople())
    {            
        if(!vch->isDead() && vch->in_room == room){

        if (vch->is_mirror() && number_percent() < 50) 
        continue;
        

        if (is_safe_spell(ch,vch,true))
            continue;

        if (ch->is_npc() && vch->is_npc()
            && ch->fighting != vch && vch->fighting != ch)
            continue;

        if (is_safe(ch, vch))
          continue;
          
        if (ch->fighting != vch && vch->fighting != ch)
            yell_panic( ch, vch );

        if (!is_flying( vch )) dam /= 2;

        if (vch->size == SIZE_TINY)  dam = ( int )( dam * 1.5 );
        else if (vch->size == SIZE_SMALL)  dam = ( int )( dam * 1.3 );
        else if (vch->size == SIZE_MEDIUM)  dam *= 1;
        else if (vch->size == SIZE_LARGE)  dam = ( int )( dam * 0.9 );
        else if (vch->size == SIZE_HUGE)  dam = ( int )( dam * 0.7 );
        else dam = ( int )( dam * 0.5 );
    try{
        if (saves_spell(level,vch,DAM_OTHER, ch, DAMF_SPELL))
            damage_nocatch(ch,vch,dam/2,sn,DAM_OTHER,true, DAMF_SPELL);
        else
            damage_nocatch(ch,vch,dam,sn,DAM_OTHER,true, DAMF_SPELL);
        }
    catch (const VictimDeathException &){
        continue;
    }
        }

    }

}




/* $Id: group_draconian.cpp,v 1.1.2.11.6.8 2010/01/01 15:48:15 rufina Exp $
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
#include "act_move.h"
#include "gsn_plugin.h"
#include "effects.h"
#include "damage.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"





SPELL_DECL(DragonsBreath);
VOID_SPELL(DragonsBreath)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        
        int dam,hp_dam,dice_dam;
        int hpch;

        act("Ты призываешь на помощь могущественную силу дракона.", ch,0,0,TO_CHAR);
        act("Дыхание $c2 приобретает силу дракона.", ch,0,victim,TO_ROOM);
        act("Ты дышишь дыханием Повелителя Драконов.", ch,0,0,TO_CHAR);

        hpch = max( 10, (int)ch->hit );
        hp_dam  = number_range( hpch/9+1, hpch/5 );

        if ( ch->is_npc( ) )
                hp_dam /= 6;

        dice_dam = dice(level,20);

        dam = max(hp_dam + dice_dam / 5, dice_dam + hp_dam / 5);

        switch( dice(1,5) )
        {
        case 1:
                fire_effect(victim->in_room,level,dam/2,TARGET_ROOM, DAMF_SPELL);

                for ( auto &vch : victim->in_room->getPeople())
                {
                        if(!vch->isDead() && vch->in_room == victim->in_room){

                        if ( vch->is_mirror()
                        && ( number_percent() < 50 ) ) continue;


                        if ( is_safe_spell(ch,vch,true)
                                || ( vch->is_npc() && ch->is_npc()
                                        && (ch->fighting != vch && vch->fighting != ch)))
                                continue;

                        if ( is_safe(ch, vch) )
                                continue;

                        if (vch == victim) /* full damage */
                        {
                                try{
                                if (saves_spell(level,vch,DAM_FIRE,ch, DAMF_SPELL))
                                {
                                        fire_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam/2,sn,DAM_FIRE,true, DAMF_SPELL);
                                }
                                else
                                {
                                        fire_effect(vch,level,dam,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam,sn,DAM_FIRE,true, DAMF_SPELL);
                                }
                                }
                                catch (const VictimDeathException &){
                                        continue;
                                }
                        }
                        else /* partial damage */
                        {
                                try{
                                if (saves_spell(level - 2,vch,DAM_FIRE,ch, DAMF_SPELL))
                                {
                                        fire_effect(vch,level/4,dam/8,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam/4,sn,DAM_FIRE,true, DAMF_SPELL);
                                }
                                else
                                {
                                        fire_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam/2,sn,DAM_FIRE,true, DAMF_SPELL);
                                }
                                }
                                catch (const VictimDeathException &){
                                        continue;
                                }
                        }
                        }
                }
    break;

        case 2:
                if (saves_spell(level,victim,DAM_ACID,ch, DAMF_SPELL))
                {
                        acid_effect(victim,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                        damage_nocatch(ch,victim,dam/2,sn,DAM_ACID,true, DAMF_SPELL);
                }
                else
                {
                        acid_effect(victim,level,dam,TARGET_CHAR, DAMF_SPELL);
                        damage_nocatch(ch,victim,dam,sn,DAM_ACID,true, DAMF_SPELL);
                }
                break;

        case 3:
                cold_effect(victim->in_room,level,dam/2,TARGET_ROOM, DAMF_SPELL);

                for ( auto &vch : victim->in_room->getPeople())
                {                        
                        if(!vch->isDead() && vch->in_room == victim->in_room){

                        if ( vch->is_mirror()
                        && ( number_percent() < 50 ) ) continue;

                        if ( is_safe_spell(ch,vch,true)
                                || ( vch->is_npc() && ch->is_npc()
                                        && (ch->fighting != vch && vch->fighting != ch)))
                                continue;

                        if ( is_safe(ch, vch) )
                                continue;

                        if (vch == victim) /* full damage */
                        {
                                try{
                                if (saves_spell(level,vch,DAM_COLD,ch, DAMF_SPELL))
                                {
                                        cold_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam/2,sn,DAM_COLD,true, DAMF_SPELL);
                                }
                                else
                                {
                                        cold_effect(vch,level,dam,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam,sn,DAM_COLD,true, DAMF_SPELL);
                                }
                                }
                                catch (const VictimDeathException &){
                                        continue;
                                }
                        }
                        else
                        {
                                try{
                                if (saves_spell(level - 2,vch,DAM_COLD,ch, DAMF_SPELL))
                                {
                                        cold_effect(vch,level/4,dam/8,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam/4,sn,DAM_COLD,true, DAMF_SPELL);
                                }
                                else
                                {
                                        cold_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                                        damage_nocatch(ch,vch,dam/2,sn,DAM_COLD,true, DAMF_SPELL);
                                }
                                }
                                catch (const VictimDeathException &){
                                        continue;
                                }
                        }
                        }
                }
                break;

        case 4:
                poison_effect(victim->in_room,level,dam,TARGET_ROOM, DAMF_SPELL);

                for ( auto &vch : victim->in_room->getPeople())
                {

                        if(!vch->isDead() && vch->in_room == victim->in_room){
       
                        if ( vch->is_mirror()
                        && ( number_percent() < 50 ) ) continue;

                        if ( is_safe_spell(ch,vch,true)
                                || ( ch->is_npc() && vch->is_npc()
                                        && (ch->fighting != vch && vch->fighting != ch)))
                                continue;

                        if ( is_safe(ch, vch) )
                                continue;
                        
                        if (ch->fighting != vch && vch->fighting != ch)
                            yell_panic( ch, vch );

                        try{
                                                    
                        if (saves_spell(level,vch,DAM_POISON,ch, DAMF_SPELL))
                        {
                                poison_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                                damage_nocatch(ch,vch,dam/2,sn,DAM_POISON,true, DAMF_SPELL);
                        }
                        else
                        {
                                poison_effect(vch,level,dam,TARGET_CHAR, DAMF_SPELL);
                                damage_nocatch(ch,vch,dam,sn,DAM_POISON,true, DAMF_SPELL);
                        }
                        }
                        catch (const VictimDeathException &){
                                continue;
                        }
                        }
                }
                break;

        case 5:
                if (saves_spell(level,victim,DAM_LIGHTNING,ch, DAMF_SPELL))
                {
                        shock_effect(victim,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                        damage_nocatch(ch,victim,dam/2,sn,DAM_LIGHTNING,true, DAMF_SPELL);
                }
                else
                {
                        shock_effect(victim,level,dam,TARGET_CHAR, DAMF_SPELL);
                        damage_nocatch(ch,victim,dam,sn,DAM_LIGHTNING,true, DAMF_SPELL);
                }
                break;
        }

}


/* $Id$
 *
 * ruffina, 2004
 */

#include "skill.h"
#include "spelltarget.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "pcharactermanager.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "npcharacter.h"
#include "object.h"

#include "dreamland.h"
#include "gsn_plugin.h"
#include "magic.h"
#include "fight.h"
#include "stats_apply.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "save.h"
#include "act.h"
#include "vnum.h"
#include "interp.h"
#include "def.h"
#include "move_utils.h"

PROF(ranger);
GSN(soothe);

/*
 * 'soothe' skill command
 */
SKILL_RUNP( soothe )
{
    Character *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument,arg);

    if ( ch->is_npc() || !gsn_soothe->usable( ch ) )
    {
        ch->pecho("Это ж надо уметь!");
        return;
    }

    if (arg[0] == '\0')
    {
        ch->pecho("Ты не поддаешься укрощению.");
        oldact_p("$c1 пытается укротить са$gмо|м|ма себя, но эта попытка с треском проваливается.",
                ch,0,0,TO_ROOM,POS_RESTING);
        return;
    }

    if ( (victim = get_char_room(ch,arg)) == 0)
    {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (!victim->is_npc())
    {
        ch->pecho("%1$^C1 не подда%1$nется|ются укрощению.", victim);
        return;
    }
    
    /* 
     * ranger soothe: remove aggression
     */
    if (ch->getProfession( ) == prof_ranger) {
        if (!IS_SET(victim->act,ACT_AGGRESSIVE))
        {
            ch->pecho("%1$^C1 обычно не агрессив%1$Gно|ен|на|ны.", victim);
            return;
        }

        ch->setWait( gsn_soothe->getBeats(ch)  );

        if (number_percent() < gsn_soothe->getEffective( ch ) + 15
                + 4 * ( ch->getModifyLevel() - victim->getModifyLevel() ) )
        {
            REMOVE_BIT(victim->act,ACT_AGGRESSIVE);
            SET_BIT(victim->affected_by,AFF_CALM);
            victim->pecho("Ты успокаиваешься.");
            oldact("Ты укрощаешь $C4.",ch,0,victim,TO_CHAR);
            oldact("$c1 укрощает $C4.",ch,0,victim,TO_NOTVICT);
            stop_fighting(victim,true);
            gsn_soothe->improve( ch, true, victim );
        }
        else
        {
            ch->pecho("Попытка укрощения не удалась.");
            oldact("$c1 пытается укротить $C4, но безуспешно.",
                    ch,0,victim,TO_NOTVICT);
            oldact("$c1 пытается укротить тебя, но безуспешно.",
                    ch,0,victim,TO_VICT);
            gsn_soothe->improve( ch, false, victim );
        }

        return;
    }
}

/*
 * 'hydroblast' spell
 */
SPELL_DECL(Hydroblast);
VOID_SPELL(Hydroblast)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;

    if (!RoomUtils::hasWaterParticles(ch->in_room)) {
         ch->pecho("Здесь недостаточно водных молекул.");
         ch->wait = 0;
         return;
    }
    
    oldact("Молекулы воды вокруг $c2 собираются вместе, образуя кулак.", ch, 0, 0, TO_ROOM);
    oldact("Молекулы воды вокруг тебя собираются вместе, образуя кулак.", ch, 0, 0, TO_CHAR);
    dam = dice( level , 14 );
    damage_nocatch(ch,victim,dam,sn,DAM_BASH,true, DAMF_MAGIC|DAMF_WATER);
}

/*
 * 'entangle' spell
 */
SPELL_DECL(Entangle);
VOID_SPELL(Entangle)::run( Character *ch, Object *grave, int sn, int level ) 
{
    int dam;
    PCharacter *victim;

    if (!RoomUtils::isNature(ch->in_room))
    {
        ch->pecho("Терновник растет только в лесу, поле, горах или на холмах.");
        return;
    }

    if (grave->pIndexData->vnum != OBJ_VNUM_GRAVE) {
        ch->pecho("Это не вампирская могила.");
        return;
    }

    victim = PCharacterManager::findPlayer( grave->getOwner( ) );

    if (!victim || !DIGGED(victim)) {
        ch->pecho("Колючий терновник опутывает могилу... но в ней никого не оказывается!");
        LogStream::sendError( ) << "Unexistent grave owner: " << grave->getOwner( ) << endl;
        return;
    }

    if (is_safe(ch, victim)) 
        return;
    
    if (number_percent( ) > ch->getSkill( sn ) ) {
        oldact("Могила покрывается цветочками и вьющимся барвинком.", ch, 0, 0, TO_ALL);
        return;
    }

    oldact("Корни терновника проникают в могилу, тревожа твой покой.", victim, 0, 0, TO_CHAR);
    oldact("Колючий терновник опутывает могилу, проникая корнями глубоко под землю!", ch, 0, 0, TO_ALL);
    oldact("Из-под земли раздается недовольное ворчание.", ch, 0, 0, TO_ALL);

    undig( victim );

    dam = number_range(level, 4 * level);
    if ( saves_spell( level, victim, DAM_PIERCE, ch, DAMF_MAGIC ) )
        dam /= 2;

    damage_nocatch(ch,victim, level,gsn_entangle,DAM_PIERCE, true, DAMF_MAGIC);
}

VOID_SPELL(Entangle)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;
    Affect todex;

   if (victim == ch)
   {
        ch->pecho("Ты задумчиво колешь себя шипом терновника в пятку. Ай!");
        return;
   }
    
   if (!RoomUtils::isNature(ch->in_room))
   {
        ch->pecho("Терновник растет только в лесу, поле, горах или на холмах.");
        return;
   }

   if (IS_SET(victim->imm_flags, IMM_PIERCE))
   {
        oldact_p("$C1 обладает иммунитетом к шипам терновника.", ch, 0,
                victim, TO_CHAR,POS_RESTING);
        return;
   }
    
   if (is_flying( victim ))
   {
        ch->pecho("Побеги терновника не смогут навредить летучему противнику.");
        return;
   }

    dam = number_range(level, 4 * level);
    victim->move -= victim->max_move / 3;
    victim->move = max( 0, (int)victim->move );
    
    if ( !victim->isAffected(gsn_entangle) )
    {
        if ( !saves_spell(level, victim, DAM_PIERCE, ch, DAMF_MAGIC) ){
            oldact("Колючий терновник прорастает сквозь землю, обвивая ноги $c2!",
                victim, 0, 0, TO_ROOM);
            oldact("Колючий терновник прорастает сквозь землю, обвивая твои ноги!",
                victim, 0, 0, TO_CHAR);

            todex.type = sn;
            todex.level = level;
            todex.duration = level / (5 * victim->size) + 1;
            todex.location = APPLY_DEX;
            todex.modifier = -1 * (level / 20 + 1);
            affect_join( victim, &todex); 
            
            dam = dam * 2;   
        }
        else {
            oldact("Колючий терновник прорастает сквозь землю, но $c1 с трудом разрывает его путы!",
                victim, 0, 0, TO_ROOM);
            oldact("Колючий терновник прорастает сквозь землю, но ты с трудом разрываешь его путы!",
                victim, 0, 0, TO_CHAR);
        }
    }
    else {
            oldact("Колючий терновник прорастает сквозь землю, больно раня ноги $c2!",
                victim, 0, 0, TO_ROOM);
            oldact("Колючий терновник прорастает сквозь землю, больно раня твои ноги!",
                victim, 0, 0, TO_CHAR);        
    }
   
    damage_nocatch(ch, victim, level, gsn_entangle, DAM_PIERCE, true, DAMF_MAGIC);
}

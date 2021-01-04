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

PROF(ranger);
PROF(druid);

/*
 * 'tame' skill command
 */
SKILL_RUNP( tame )
{
    Character *victim;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument,arg);

    if ( ch->is_npc() || !gsn_tame->usable( ch ) )
    {
        ch->send_to("Это ж надо уметь!\n\r");
        return;
    }

    if (arg[0] == '\0')
    {
        ch->send_to("Ты не поддаешься дрессировке.\n\r");
        act_p("$c1 пытается приручить са$gмо|м|ма себя, но эта попытка с треском проваливается.",
                ch,0,0,TO_ROOM,POS_RESTING);
        return;
    }

    if ( (victim = get_char_room(ch,arg)) == 0)
    {
        ch->send_to("Этого нет здесь.\n\r");
        return;
    }

    if (!victim->is_npc())
    {
        ch->pecho("%1$^C1 не подда%1$nется|ются дрессировке.", victim);
        return;
    }
    
    /*
     * druidic tame: control animal attracted by magic bite
     */
    if (ch->getProfession( ) == prof_druid) {
#if 0        
        DruidSummonedAnimal::Pointer animal;
        int chance;

        if (!ch->getNPC( )->behavior 
            || !(animal = ch->getNPC( )->behavior.getDynamicPointer<DruidSummonedAnimal>( ))
            || !animal->myHero( ch ))
        {
            ch->println("Это существо не поддастся твоему контролю.");
            return;
        }

        if (is_safe(ch, victim)) 
            return;
            
        if (overcharmed( ch ))
            return;

        ch->setWait( gsn_tame->getBeats( )  );

        chance = gsn_tame->getEffective( ch );
        chance += 3 * (ch->getModifyLevel( ) - victim->getModifyLevel( ));
        chance += (ch->getCurrStat(STAT_CHA) - 20) * 2;
        chance = (chance * animal->biteQuality) / 100;
        
        if (number_percent( ) > chance) {
            act("$c1 раздраженно рычит и атакует!", victim, 0, 0, TO_ROOM);
            gsn_tame->improve( ch, false, victim );
            interpret_raw(victim, "murder", ch->getNameP( ));
            return;
        }
        
        ch->add_follower( victim );
        victim->leader = ch;

        af.bitvector.setTable(&affect_flags);
        af.type      = gsn_tame;
        af.level     = ch->getModifyLevel( );
        af.duration  = -1;
        af.bitvector.setValue(AFF_CHARM);
        affect_to_char( victim, &af );
        
        act("$C1 теперь полностью подчиняется твоей воле.", ch, 0, victim, TO_CHAR);
        act("$C1 преданно смотрит в глаза $c3.", ch, 0, victim, TO_NOTVICT);
        act("Ты преданно смотришь в глаза $c3.", ch, 0, victim, TO_VICT);
        gsn_tame->improve( ch, true, victim );
#endif        
        return;
    }
    
    /* 
     * ranger tame: remove aggression
     */
    if (ch->getProfession( ) == prof_ranger) {
        if (!IS_SET(victim->act,ACT_AGGRESSIVE))
        {
            ch->pecho("%1$^C1 обычно не аггресив%1$Gно|ен|на|ны.", victim);
            return;
        }

        ch->setWait( gsn_tame->getBeats( )  );

        if (number_percent() < gsn_tame->getEffective( ch ) + 15
                + 4 * ( ch->getModifyLevel() - victim->getModifyLevel() ) )
        {
            REMOVE_BIT(victim->act,ACT_AGGRESSIVE);
            SET_BIT(victim->affected_by,AFF_CALM);
            victim->println("Ты успокаиваешься.");
            act("Ты успокаиваешь $C4.",ch,0,victim,TO_CHAR);
            act("$c1 успокаивает $C4.",ch,0,victim,TO_NOTVICT);
            stop_fighting(victim,true);
            gsn_tame->improve( ch, true, victim );
        }
        else
        {
            ch->println("Попытка не удалась.");
            act("$c1 пытается успокоить $C4, но попытка безуспешна.",
                    ch,0,victim,TO_NOTVICT);
            act("$c1 пытается успокоить тебя, но попытка безуспешна.",
                    ch,0,victim,TO_VICT);
            gsn_tame->improve( ch, false, victim );
        }

        return;
    }
}

static bool has_water_around( Character *ch )
{
    if (IS_WATER(ch->in_room))
        return true;

    if (ch->in_room->getSectorType() == SECT_UNDERWATER)
        return true;

    if (IS_SET(ch->in_room->room_flags, ROOM_NEAR_WATER))
        return true;

    if (IS_OUTSIDE(ch) && weather_info.sky >= SKY_RAINING)
        return true;

    return false;
}

/*
 * 'hydroblast' spell
 */
SPELL_DECL(Hydroblast);
VOID_SPELL(Hydroblast)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;

    if (!has_water_around( ch )) {
         ch->send_to("Здесь недостаточно водных молекул.\n\r");
         ch->wait = 0;
         return;
    }
    
    act("Молекулы воды вокруг $c2 собираются вместе, образуя кулак.", ch, 0, 0, TO_ROOM);
    act("Молекулы воды вокруг тебя собираются вместе, образуя кулак.", ch, 0, 0, TO_CHAR);
    dam = dice( level , 14 );
    damage_nocatch(ch,victim,dam,sn,DAM_BASH,true, DAMF_SPELL|DAMF_WATER);
}

/*
 * 'entangle' spell
 */
SPELL_DECL(Entangle);
VOID_SPELL(Entangle)::run( Character *ch, Object *grave, int sn, int level ) 
{
    int dam;
    PCharacter *victim;

    if (ch->in_room->getSectorType() == SECT_INSIDE ||
        ch->in_room->getSectorType() == SECT_CITY ||
        ch->in_room->getSectorType() == SECT_DESERT ||
        ch->in_room->getSectorType() == SECT_WATER_NOSWIM ||
        ch->in_room->getSectorType() == SECT_AIR)
    {
        ch->send_to("Ни одно растение не сможет здесь расти.\n\r");
        return;
    }

    if (grave->pIndexData->vnum != OBJ_VNUM_GRAVE) {
        ch->send_to("Нет никакого смысла опутывать это терновником.\r\n");
        return;
    }

    victim = PCharacterManager::findPlayer( grave->getOwner( ) );

    if (!victim || !DIGGED(victim)) {
        ch->send_to("Опс.. а могила-то ничейная..\r\n");
        LogStream::sendError( ) << "Unexistent grave owner: " << grave->getOwner( ) << endl;
        return;
    }

    if (number_percent( ) > ch->getSkill( sn ) || is_safe_nomessage( ch, victim)) {
        act_p("Могила покрывается цветочками и вьющимся барвинком.", ch, 0, 0, TO_ALL, POS_RESTING);
        return;
    }

    act_p("Корни терновника проникают в могилу, тревожа твой покой.", victim, 0, 0, TO_CHAR, POS_RESTING);
    act_p("Колючий терновник опутывает могилу, проникая корнями глубоко под землю!", ch, 0, 0, TO_ALL, POS_RESTING);
    act_p("Из-под земли раздается недовольное ворчание.", ch, 0, 0, TO_ALL, POS_RESTING);

    undig( victim );

    dam = number_range(level, 4 * level);
    if ( saves_spell( level, victim, DAM_PIERCE, ch, DAMF_SPELL ) )
        dam /= 2;

    damage_nocatch(ch,victim, ch->getModifyLevel(),gsn_entangle,DAM_PIERCE, true, DAMF_SPELL);
}

VOID_SPELL(Entangle)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    int dam;
    Affect todex;

    if (ch->in_room->getSectorType() == SECT_INSIDE ||
        ch->in_room->getSectorType() == SECT_CITY ||
        ch->in_room->getSectorType() == SECT_DESERT ||
        ch->in_room->getSectorType() == SECT_WATER_NOSWIM ||
        ch->in_room->getSectorType() == SECT_AIR)
    {
        ch->send_to("Ни одно растение не сможет здесь расти.\n\r");
        return;
    }

    act("Колючий терновник прорастает сквозь землю, обвивая ноги $c2!",
        victim, 0, 0, TO_ROOM);
    act("Колючий терновник прорастает сквозь землю, обвивая твои ноги!",
        victim, 0, 0, TO_CHAR);
        
    dam = number_range(level, 4 * level);
    if (saves_spell( level, victim, DAM_PIERCE, ch, DAMF_SPELL ))
        dam /= 2;

    victim->move -= victim->max_move / 3;
    victim->move = max( 0, (int)victim->move );
    
    todex.type = sn;
    todex.level = level;
    todex.duration = level / 10;
    todex.location = APPLY_DEX;
    todex.modifier = -1;
    affect_join( victim, &todex);

    damage_nocatch(ch, victim, ch->getModifyLevel(), gsn_entangle, DAM_PIERCE, true, DAMF_SPELL);
}


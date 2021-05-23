/* $Id: class_warlock.cpp,v 1.1.2.11.6.16 2009/09/01 22:29:51 rufina Exp $
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
#include "class_warlock.h"

#include "playerattributes.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "desire.h"
#include "object.h"

#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "handler.h"
#include "effects.h"
#include "damage_impl.h"
#include "act.h"
#include "merc.h"
#include "interp.h"
#include "def.h"

GSN(shocking_trap);

#define OBJ_VNUM_FIRE_SHIELD        92

/*
 * 'blink' skill command
 */

SKILL_RUNP( blink )
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument , arg);

    if (!ch->is_npc() && !gsn_blink->usable( ch ))
    {
        ch->pecho("Это умение тебе недоступно.");
        return;
    }

    if (arg[0] == '\0' )
    {
        ch->printf("Во время боя ты {W%sмерцаешь{x.\n\r",
                    IS_SET(ch->act, PLR_BLINK_ON) ? "" : "не ");
        return;
    }

    if (arg_is_switch_on( arg ))
        {
            ch->pecho("Ты будешь мерцать, уклоняясь от атак.");
            SET_BIT(ch->act,PLR_BLINK_ON);
             return;
        }

    if (arg_is_switch_off( arg ))
        {
         REMOVE_BIT(ch->act,PLR_BLINK_ON);
         ch->pecho("Ты больше не будешь мерцать, уклоняясь от атак.");
         return;
        }
    
    ch->pecho("Укажи {lRвкл или выкл{lEon или off{lx в качестве аргумента."); 
}

SPELL_DECL(Disintegrate);
VOID_SPELL(Disintegrate)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        int dam=0;

        if ( victim->fighting )
        {
                ch->pecho("Ты не можешь прицелиться, жертва слишком быстро движется.");
                return;
        }

        short chance = 50;

        if ( !victim->is_npc() )
                chance /= 2;

        if ( saves_spell(level,victim,DAM_MENTAL,ch, DAMF_MAGIC) )
                chance = 0;

        ch->setWait( skill->getBeats(ch) );
        
        if ( !ch->is_immortal()
                && ( victim->is_immortal()
                        || number_percent() > chance ) )
        {
                dam = dice( level , 24 ) ;
                damage_nocatch(ch, victim , dam , sn, DAM_MENTAL, true, DAMF_MAGIC);
                return;
        }

        oldact_p("$C1 разрушающим световым лучом {R###ПОЛНОСТЬЮ УНИЧТОЖАЕТ###{x тебя!",
                victim, 0, ch, TO_CHAR, POS_RESTING);
        oldact_p("$c1 разрушающим световым лучом {R###ПОЛНОСТЬЮ УНИЧТОЖАЕТ###{x $C4!",
                ch, 0, victim, TO_NOTVICT, POS_RESTING);
        oldact_p("Разрушающим световым лучом ты {R###ПОЛНОСТЬЮ УНИЧТОЖАЕШЬ###{x $C4!",
                ch, 0, victim, TO_CHAR, POS_RESTING);
        victim->pecho("Тебя {RУБИЛИ{x!");

        oldact("Тебя больше не существует!\n\r", victim, 0, 0, TO_CHAR);
        oldact("$c2 больше не существует!\n\r", victim, 0, 0, TO_ROOM);

        victim->pecho("{YБожественные Силы возвращают тебя к жизни!{x");
        
        group_gain( ch, victim );
        raw_kill( victim, -1, ch, FKILL_REABILITATE | FKILL_PURGE | FKILL_MOB_EXTRACT );
        pk_gain( ch, victim );
        
        victim->hit  = 1;
        victim->mana = 1;
}


SPELL_DECL(Scream);
VOID_SPELL(Scream)::run( Character *ch, Room *room, int sn, int level ) 
{ 

        int dam=0,hp_dam,dice_dam;
        int hpch;

        if ( ch->isAffected(sn ) )
        {
            ch->pecho("Ты пытаешься крикнуть, но только хрип вырывается из твоего горла.");
            oldact("$c1 хрипит!",ch,0,0,TO_ROOM);
            return;
        }

        oldact_p("$c1 пронзительно кричит, сотрясая все вокруг!",
                ch,0,0,TO_ROOM,POS_RESTING);
        oldact_p("Ты пронзительно кричишь, сотрясая все вокруг.",
                ch,0,0,TO_CHAR,POS_RESTING);

        hpch = max( 10, (int)ch->hit );
        if ( ch->is_npc() )
                hpch /= 6;
        hp_dam  = number_range( hpch/9+1, hpch/5 );
        dice_dam = dice(level,20);
        dam = max(hp_dam + dice_dam /10 , dice_dam + hp_dam /10);

        scream_effect(room,level,dam/2,TARGET_ROOM, DAMF_MAGIC);

        for ( auto &vch : room->getPeople())
        {
            if(!vch->isDead() && vch->in_room == room){

                if (is_safe_spell(ch,vch,true))
                        continue;

                if ( is_safe(ch, vch) )
                        continue;

                if ( ch != vch && !saves_spell(level,vch,DAM_SOUND,ch, DAMF_MAGIC))
                        vch->setWaitViolence( 2 );
                        
                if (saves_spell(level,vch,DAM_SOUND,ch, DAMF_MAGIC))
                        scream_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_MAGIC);
                else
                        scream_effect(vch,level,dam,TARGET_CHAR, DAMF_MAGIC);
            }
       }

}


SPELL_DECL(Shielding);
VOID_SPELL(Shielding)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (saves_spell( level, victim, DAM_OTHER,ch, DAMF_MAGIC)) {
        if (ch != victim)
            oldact_p("Легкая дрожь пронизывает $C4, но это быстро проходит.",
               ch, 0, victim, TO_CHAR,POS_RESTING );
        victim->pecho("Легкая дрожь пронизывает тебя, но это быстро проходит.");
        return;
    }

    if (!victim->isAffected(sn)) {
        af.type    = sn;
        af.level   = level;
        af.duration = level / 20;
        affect_to_char(victim, &af );
        if (ch != victim)
            oldact("Ты создаешь изолирующий магию экран вокруг $C2.", ch, 0, victim, TO_CHAR);
        victim->pecho("Вокруг тебя внезапно воздвигается экран, изолирующий магию!");
    }
    else {
        af.type        = sn;
        af.level    = level;
        af.duration = level / 15;
        affect_join( victim, &af );

        victim->pecho("Изолирующий магию экран вокруг тебя усиливается!");
        if (ch != victim)
            oldact("Изолирующий магию экран вокруг $C2 усиливается.", ch, 0, victim, TO_CHAR);
    }
}

SPELL_DECL(ShockingTrap);
VOID_SPELL(ShockingTrap)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
        ch->pecho("Комната уже наполнена силовыми волнами.");
        return;
    }

    if ( ch->isAffected(sn))
    {
        ch->pecho("Это заклинание использовалось совсем недавно.");
        return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 40;
    
    af.modifier  = 0;
    af.bitvector.setValue(AFF_ROOM_SHOCKING);
    room->affectTo( &af );

    postaffect_to_char( ch, sn, level / 10 );

    ch->pecho("Комната наполняется силовыми волнами, заставляя вибрировать воздух.");
    oldact_p("$c1 заставляет вибрировать воздух, наполняя комнату силовыми волнами.",
           ch,0,0,TO_ROOM,POS_RESTING);
}

struct ShockingTrapDamage : public SelfDamage {
    ShockingTrapDamage( Character *ch, int dam ) : SelfDamage( ch, DAM_LIGHTNING, dam )
    {
    }
    virtual void message( ) {
        msgRoom( "%2$^O1, наполняющие местность,\6%3$C4", dam, gsn_shocking_trap->getDammsg(), ch );
        msgChar( "%2$^O1, наполняющие местность,\6тебя", dam, gsn_shocking_trap->getDammsg(), ch );
    }
};
        
AFFECT_DECL(ShockingTrap);
VOID_AFFECT(ShockingTrap)::entry( Room *room, Character *ch, Affect *paf )
{
    if (!is_safe_rspell(paf->level,ch)) {
        try {
            ShockingTrapDamage( ch, dice(paf->level,4)+12 ).hit( true );
        }
        catch (const VictimDeathException &) {
        }
        room->affectRemove( paf);
    }
}

VOID_AFFECT(ShockingTrap)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Воздух вибрирует от переизбытка энергии, это продлится еще {W%1$d{x ча%1$Iс|са|сов.",
                   paf->duration )
        << endl;
}

SPELL_DECL(WitchCurse);
VOID_SPELL(WitchCurse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (victim->isAffected(gsn_witch_curse)) {
        ch->pecho("На этой жертве уже лежит ведьминское проклятие.");
        return;
    }

    ch->hit -=(2 * level);

    af.type             = gsn_witch_curse;
    af.level            = level;
    af.duration         = 24;
    af.location = APPLY_HIT;
    af.modifier         = - level;
    af.sources.add(ch);
    affect_to_char(victim,&af);

    oldact("$C1 вступи$Gло|л|ла на путь смерти.", ch, 0, victim, TO_CHAR);
    oldact("$C1 вступи$Gло|л|ла на путь смерти.", ch, 0, victim, TO_NOTVICT);
    oldact("Ты вступи$Gло|л|ла на путь смерти.",  ch, 0, victim, TO_VICT);
}

AFFECT_DECL(WitchCurse);
VOID_AFFECT(WitchCurse)::update( Character *ch, Affect *paf ) 
{
    Affect witch;
    
    DefaultAffectHandler::update( ch, paf );

    oldact_p("Проклятие ведьм безжалостно отбирает жизнь у $c2.",
          ch,0,0,TO_ROOM,POS_RESTING);
    ch->pecho("{RПроклятие ведьм безжалостно отбирает у тебя жизнь!{x");

    if (paf->level <= 1)
        return;

    witch.type  = paf->type;
    witch.level = paf->level;
    witch.duration = paf->duration;
    witch.location.setTable(paf->location.getTable());
    witch.location = paf->location;
    witch.modifier = paf->modifier * 2;
    witch.sources = paf->sources;

    affect_remove(ch, paf);
    affect_to_char( ch ,&witch);
    ch->hit = min(ch->hit,ch->max_hit);

    if (ch->hit < 1) {
        affect_strip(ch,gsn_witch_curse);
        damage_nocatch(&witch,ch,20,gsn_witch_curse,DAM_NONE,false);
    }
}


SPELL_DECL(LightningShield);
VOID_SPELL(LightningShield)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
        ch->pecho("Эта комната уже защищена щитом молний.");
        return;
    }

    if ( ch->isAffected(sn))
    {
        ch->pecho("Это заклинание использовалось совсем недавно.");
        return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 40;
    
    af.modifier  = 0;
    af.bitvector.setValue(AFF_ROOM_L_SHIELD);
    room->affectTo( &af );

    postaffect_to_char( ch, sn, level / 10 );

    ch->in_room->owner = str_dup( ch->getNameC() );
    ch->pecho("Ты воздвигаешь вокруг себя щит молний.");
    oldact("$c1 окружает себя щитом молний.",ch,0,0,TO_ROOM);
    return;

}

AFFECT_DECL(LightningShield);
VOID_AFFECT(LightningShield)::entry( Room *room, Character *ch, Affect *paf )
{
    Character *vch;

    for (vch=room->people;vch;vch=vch->next_in_room)
        if (room->isOwner(vch)) 
            break;

    if ( !vch ) {
        bug("Owner of lightning shield left the room.",0);
        free_string(room->owner);
        room->owner = str_dup("");        
        room->affectStrip(paf->type);
    }
    else if (!ch->is_immortal( )) {
        ch->pecho("Щит молний, оберегающий это место, блокирует тебя.");
        oldact("$C1 приближается к тебе.",vch,0,ch,TO_CHAR);
        interpret_raw( vch, "wake" );

        if (!is_safe_rspell(paf->level,ch)) {
            int level = paf->level;
            int sn = paf->type;

            free_string(room->owner);
            room->owner = str_dup("");        
            room->affectRemove(paf);

            damage_nocatch(vch, ch, dice(level, 4) + 12, sn, DAM_LIGHTNING, true, DAMF_MAGIC);
        }
    }
}

VOID_AFFECT(LightningShield)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Здесь установлен щит молний, который просуществует еще {W%1$d{x ча%1$Iс|са|сов.",
                   paf->duration )
        << endl;
}

VOID_AFFECT(LightningShield)::leave( Room *room, Character *ch, Affect *paf )
{
    if (room->isOwner(ch)) {
        free_string(room->owner);
        room->owner = str_dup("");
        room->affectStrip( paf->type );
    }
}

/*
 * energy shield behavior
 */
bool EnergyShield::isColdShield( ) const
{
  return (obj->extra_descr
                && obj->extra_descr->description
                && strstr( obj->extra_descr->description, "холод" ) != 0 );
}
bool EnergyShield::isFireShield( ) const
{
  return (obj->extra_descr
                && obj->extra_descr->description
                && strstr( obj->extra_descr->description, "огня" ) != 0 );
}
void EnergyShield::wear( Character *ch )
{
    if (!ch->isAffected(gsn_make_shield)) {
        if (isColdShield( ))
            ch->pecho("Твоя сопротивляемость холоду повышается.");
        else if (isFireShield( )) 
            ch->pecho("Твоя сопротивляемость огню повышается.");
    }
}

void EnergyShield::equip( Character *ch )
{
    Affect af;
    
    if (ch->isAffected(gsn_make_shield))
        return;

    af.bitvector.setTable(&res_flags);
    af.type = gsn_make_shield;
    af.duration = -2;
    af.level = ch->getModifyLevel();

    if (isColdShield( ))
        af.bitvector.setValue(RES_COLD);
    else if (isFireShield( )) 
        af.bitvector.setValue(RES_FIRE);
    else
        return;
   
    affect_to_char(ch, &af);
}

void EnergyShield::remove( Character *ch )
{
    if (!ch->isAffected(gsn_make_shield))
        return;

    affect_strip(ch, gsn_make_shield);

    if (isColdShield( ))
        ch->pecho("Твоя сопротивляемость холоду становится хуже.");
    else  if (isFireShield( ))
        ch->pecho("Твоя сопротивляемость огню становится хуже.");
}

/*
 * 'make shield' spell
 */
SPELL_DECL(MakeShield);
VOID_SPELL(MakeShield)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    Object *fire;

    target_name = one_argument( target_name, arg );
    DLString from;

    if (arg_oneof(arg, "cold", "холода"))
        from = "холода";
    else if (arg_oneof(arg, "fire", "огонь", "огня"))
        from = "огня";
    else {
        ch->pecho("От чего ты хочешь защититься? Укажи {lrогонь или холод{lefire или cold{x в качестве параметра.");
        return;
    }
        
    fire        = create_object(get_obj_index(OBJ_VNUM_FIRE_SHIELD), 0);
    fire->setOwner(ch->getNameC());
    fire->from = str_dup(ch->getNameC());
    fire->level = ch->getRealLevel( );

    sprintf( buf, fire->pIndexData->extra_descr->description, from.c_str() );
    fire->extra_descr = new_extra_descr();
    fire->extra_descr->keyword =
              str_dup( fire->pIndexData->extra_descr->keyword );
    fire->extra_descr->description = str_dup( buf );
    fire->extra_descr->next = 0;

    fire->level = ch->getRealLevel( );
    fire->cost = 0;
    fire->timer = 5 * ch->getModifyLevel();

    if (IS_GOOD(ch))
         SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
    else if (IS_NEUTRAL(ch))
         SET_BIT(fire->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
    else if (IS_EVIL(ch))
         SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));        
         
    obj_to_char( fire, ch);
    ch->printf("Ты создаешь энергетический щит для защиты от %s.\n\r", from.c_str());
}


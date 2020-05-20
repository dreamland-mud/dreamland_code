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
        ch->send_to("Что?\n\r");
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
            ch->println("Ты будешь мерцать, уклоняясь от атак.");
            SET_BIT(ch->act,PLR_BLINK_ON);
             return;
        }

    if (arg_is_switch_off( arg ))
        {
         REMOVE_BIT(ch->act,PLR_BLINK_ON);
         ch->println("Ты больше не будешь мерцать, уклоняясь от атак.");
         return;
        }
    
    ch->println("Укажи {lRвкл или выкл{lEon или off{lx в качестве аргумента."); 
}

SPELL_DECL(Disintegrate);
VOID_SPELL(Disintegrate)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        int dam=0;

        if ( victim->fighting )
        {
                ch->send_to("Ты не можешь сосредоточиться.. Жертва слишком быстро движется.\n\r");
                return;
        }

        short chance = 50;

        if ( !victim->is_npc() )
                chance /= 2;

        if ( saves_spell(level,victim,DAM_MENTAL,ch, DAMF_SPELL) )
                chance = 0;

        ch->setWait( skill->getBeats( ) );
        
        if ( !ch->is_immortal()
                && ( victim->is_immortal()
                        || number_percent() > chance ) )
        {
                dam = dice( level , 24 ) ;
                damage_nocatch(ch, victim , dam , sn, DAM_MENTAL, true, DAMF_SPELL);
                return;
        }

        act_p("$C1 разрушающим световым лучом {R###ПОЛНОСТЬЮ УНИЧТОЖАЕТ###{x тебя!",
                victim, 0, ch, TO_CHAR, POS_RESTING);
        act_p("$c1 разрушающим световым лучом {R###ПОЛНОСТЬЮ УНИЧТОЖАЕТ###{x $C4!",
                ch, 0, victim, TO_NOTVICT, POS_RESTING);
        act_p("Разрушающим световым лучом ты {R###ПОЛНОСТЬЮ УНИЧТОЖАЕШЬ###{x $C4!",
                ch, 0, victim, TO_CHAR, POS_RESTING);
        victim->send_to("Тебя {RУБИЛИ{x!\n\r");

        act_p("Тебя больше не существует!\n\r", victim, 0, 0, TO_CHAR,POS_RESTING);
        act_p("$c2 больше не существует!\n\r", victim, 0, 0, TO_ROOM,POS_RESTING);

        victim->send_to("{YБожественные Силы возвращают тебя к жизни!{x\n\r");
        
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
            ch->send_to("Ты пытаешься крикнуть, но только хрип вырывается из твоего горла.");
            act_p("$c1 хрипит!",ch,0,0,TO_ROOM,POS_RESTING);
            return;
        }

        act_p("$c1 пронзительно кричит, сотрясая все вокруг!",
                ch,0,0,TO_ROOM,POS_RESTING);
        act_p("Ты пронзительно кричишь, сотрясая все вокруг.",
                ch,0,0,TO_CHAR,POS_RESTING);

        hpch = max( 10, (int)ch->hit );
        if ( ch->is_npc() )
                hpch /= 6;
        hp_dam  = number_range( hpch/9+1, hpch/5 );
        dice_dam = dice(level,20);
        dam = max(hp_dam + dice_dam /10 , dice_dam + hp_dam /10);

        scream_effect(room,level,dam/2,TARGET_ROOM, DAMF_SPELL);

        for ( auto &vch : room->getPeople())
        {
                if (is_safe_spell(ch,vch,true))
                        continue;

                if ( is_safe(ch, vch) )
                        continue;

                if ( ch != vch && !saves_spell(level,vch,DAM_SOUND,ch, DAMF_SPELL))
                        vch->setWaitViolence( 2 );
                        
                if (saves_spell(level,vch,DAM_SOUND,ch, DAMF_SPELL))
                        scream_effect(vch,level/2,dam/4,TARGET_CHAR, DAMF_SPELL);
                else
                        scream_effect(vch,level,dam,TARGET_CHAR, DAMF_SPELL);
        }

}


SPELL_DECL(Shielding);
VOID_SPELL(Shielding)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if (saves_spell( level, victim, DAM_OTHER,ch, DAMF_SPELL)) {
        act_p("Легкая дрожь пронизывает $C4, но это быстро проходит.",
               ch, 0, victim, TO_CHAR,POS_RESTING );
        victim->send_to("Легкая дрожь пронизывает тебя, но это быстро проходит.\n\r");
        return;
    }

    if (!victim->isAffected(sn)) {
        af.type    = sn;
        af.level   = level;
        af.duration = level / 20;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = 0;
        affect_to_char(victim, &af );
        if (ch != victim)
            act_p("Ты создаешь экран Магической Силы вокруг $C2.", ch, 0, victim, TO_CHAR,POS_RESTING);
        victim->send_to("Магическая Сила создает экран вокруг тебя.\n\r");
    }
    else {
        af.type        = sn;
        af.level    = level;
        af.duration = level / 15;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = 0;
        affect_join( victim, &af );

        victim->send_to("Магическая Сила полностью изолирует тебя от внешнего мира.\n\r");
        if (ch != victim)
            act_p("Магическая Сила полностью изолирует $C4 от внешнего мира.", ch, 0, victim, TO_CHAR,POS_RESTING);
    }
}

SPELL_DECL(ShockingTrap);
VOID_SPELL(ShockingTrap)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
        ch->send_to("Комната уже наполнена силовыми волнами.\n\r");
        return;
    }

    if ( ch->isAffected(sn))
    {
        ch->send_to("Это заклинание использовалось совсем недавно.\n\r");
        return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 40;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_SHOCKING;
    room->affectTo( &af );

    postaffect_to_char( ch, sn, level / 10 );

    ch->send_to("Комната наполняется силовыми волнами, заставляя вибрировать воздух.\n\r");
    act_p("$c1 заставляет вибрировать воздух, наполняя комнату силовыми волнами.",
           ch,0,0,TO_ROOM,POS_RESTING);
}

struct ShockingTrapDamage : public SelfDamage {
    ShockingTrapDamage( Character *ch, int dam ) : SelfDamage( ch, DAM_LIGHTNING, dam )
    {
    }
    virtual void message( ) {
        msgRoom( "%1$^O1, наполняющие местность,\6%2$C4", gsn_shocking_trap->getDammsg(), ch );
        msgChar( "%1$^O1, наполняющие местность,\6тебя", gsn_shocking_trap->getDammsg(), ch );
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
        ch->println("Твой противник уже проклят ведьмами.");
        return;
    }

    ch->hit -=(2 * level);

    af.where                = TO_AFFECTS;
    af.type             = gsn_witch_curse;
    af.level            = level;
    af.duration         = 24;
    af.location         = APPLY_HIT;
    af.modifier         = - level;
    af.bitvector        = 0;
    affect_to_char(victim,&af);

    act("$C1 вступи$Gло|л|ла на путь смерти.", ch, 0, victim, TO_CHAR);
    act("$C1 вступи$Gло|л|ла на путь смерти.", ch, 0, victim, TO_NOTVICT);
    act("Ты вступи$Gло|л|ла на путь смерти.",  ch, 0, victim, TO_VICT);
}

AFFECT_DECL(WitchCurse);
VOID_AFFECT(WitchCurse)::update( Character *ch, Affect *paf ) 
{
    Affect witch;
    
    DefaultAffectHandler::update( ch, paf );

    act_p("Проклятие ведьм безжалостно отбирает жизнь у $c2.",
          ch,0,0,TO_ROOM,POS_RESTING);
    ch->send_to("Проклятие ведьм безжалостно отбирает у тебя жизнь.\n\r");

    if (paf->level <= 1)
        return;

    witch.where = paf->where;
    witch.type  = paf->type;
    witch.level = paf->level;
    witch.duration = paf->duration;
    witch.location = paf->location;
    witch.modifier = paf->modifier * 2;
    witch.bitvector = 0;

    affect_remove(ch, paf);
    affect_to_char( ch ,&witch);
    ch->hit = min(ch->hit,ch->max_hit);

    if (ch->hit < 1) {
        affect_strip(ch,gsn_witch_curse);
        damage_nocatch(ch,ch,20,gsn_witch_curse,DAM_NONE,false);
    }
}


SPELL_DECL(LightningShield);
VOID_SPELL(LightningShield)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( room->isAffected( sn ))
    {
        ch->send_to("Эта комната уже защищена щитом.\n\r");
        return;
    }

    if ( ch->isAffected(sn))
    {
        ch->send_to("Это заклинание использовалось совсем недавно.\n\r");
        return;
    }

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 40;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_L_SHIELD;
    room->affectTo( &af );

    postaffect_to_char( ch, sn, level / 10 );

    ch->in_room->owner = str_dup( ch->getNameP( ) );
    ch->send_to("Комната наполняется молниями.\n\r");
    act_p("$c1 окружает себя молниями.",ch,0,0,TO_ROOM,POS_RESTING);
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
        ch->send_to("Защитный щит комнаты блокирует тебя.\n\r");
        act_p("$C1 заходит в комнату.",vch,0,ch,TO_CHAR,POS_RESTING);
        interpret_raw( vch, "wake" );

        if (!is_safe_rspell(paf->level,ch)) {
            free_string(room->owner);
            room->owner = str_dup("");        
            room->affectRemove(paf);
            damage_nocatch( vch,ch,dice(paf->level,4)+12, paf->type,DAM_LIGHTNING, true, DAMF_SPELL);
        }
    }
}

VOID_AFFECT(LightningShield)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Здесь установлен волшебный щит, который просуществует еще {W%1$d{x ча%1$Iс|са|сов.",
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
            ch->send_to("Твоя сопротивляемость холоду повышается.\n\r");
        else if (isFireShield( )) 
            ch->send_to("Твоя сопротивляемость огню повышается.\n\r");
    }
}

void EnergyShield::equip( Character *ch )
{
    Affect af;
    
    if (ch->isAffected(gsn_make_shield))
        return;

    af.where = TO_RESIST;
    af.type = gsn_make_shield;
    af.duration = -2;
    af.level = ch->getModifyLevel();

    if (isColdShield( ))
        af.bitvector = RES_COLD;
    else if (isFireShield( )) 
        af.bitvector = RES_FIRE;
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
        ch->send_to("Твоя сопротивляемость холоду становится хуже.\n\r");
    else  if (isFireShield( ))
        ch->send_to("Твоя сопротивляемость огню становится хуже.\n\r");
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
        ch->send_to("Выбери: от огня или холода он будет тебя защищать.\n\r");
        return;
    }
        
    fire        = create_object(get_obj_index(OBJ_VNUM_FIRE_SHIELD), 0);
    fire->setOwner(ch->getNameP( ));
    fire->from = str_dup(ch->getNameP( ));
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


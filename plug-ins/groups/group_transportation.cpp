
/* $Id: group_transportation.cpp,v 1.1.2.17.6.14 2010-08-24 20:23:09 rufina Exp $
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
#include "affecthandlertemplate.h"
#include "transportspell.h"
#include "recallmovement.h"
#include "profflags.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "clanreference.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

CLAN(none);
PROF(samurai);

#define OBJ_VNUM_PORTAL                     25
#define OBJ_VNUM_HOLY_PORTAL             34 

/*
 * 'mental block' spell
 */
SPELL_DECL(MentalBlock);
VOID_SPELL(MentalBlock)::run( Character *ch, Character *victim, int sn, int level ) 
{
    Affect af;

    if (victim->isAffected( sn )) {
        victim->println( "Ты и так блокируешь все попытки ментального контакта." );
        
        if (ch != victim)
            act( "$C1 и так блокирует все попытки ментального контакта.", ch, 0, victim, TO_CHAR );

        return;
    }
    
    af.type  = sn;
    af.where = TO_AFFECTS;
    af.level = level;
    af.duration = level / 2;
    affect_to_char( victim, &af ); 
    
    victim->println( "Теперь ты будешь блокировать все попытки ментального контакта с тобой." );

    if (ch != victim)
        act( "$C1 будет блокировать все попытки ментального контакта.", ch, 0, victim, TO_CHAR );
}

/*
 * 'fly' spell
 */
SPELL_DECL(Fly);
VOID_SPELL(Fly)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;

        if (is_flying( victim ))
        {
                if (victim == ch)
                        ch->send_to("Ты уже находишься в воздухе.\n\r");
                else
                        act_p("$C1 уже находится в воздухе.",ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }

        if (can_fly( victim ))
        {
                if (victim == ch)
                        ch->send_to("Ты и так можешь подняться в воздух.\n\r");
                else
                        act_p("$C1 может подняться в воздух и без твоей помощи.",ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }

        af.where     = TO_AFFECTS;
        af.type      = sn;
        af.level         = level;
        af.duration  = level + 3;
        af.location  = 0;
        af.modifier  = 0;
        af.bitvector = AFF_FLYING;
        affect_to_char( victim, &af );

        victim->send_to("Твои ноги отрываются от земли.\n\r");

        act_p( "$c1 поднимается в воздух.", victim, 0, 0, TO_ROOM,POS_RESTING);

        return;

}

AFFECT_DECL(Fly);
VOID_AFFECT(Fly)::remove( Character *victim ) 
{
    if (victim->posFlags.isSet( POS_FLY_DOWN )) 
        victim->println("Ты теряешь способность к полетам.");
    else
        DefaultAffectHandler::remove( victim );                                     

}

VOID_AFFECT(Fly)::dispel( Character *victim ) 
{
    if (victim->posFlags.isSet( POS_FLY_DOWN )) 
        victim->recho("%^C1 теряет способность к полетам.", victim);
    else
        DefaultAffectHandler::dispel( victim );                                     
}

/*
 * 'pass door' spell
 */
SPELL_DECL(PassDoor);
VOID_SPELL(PassDoor)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_PASS_DOOR) )
    {
        if (victim == ch)
          ch->send_to("Ты уже можешь проходить сквозь преграды.\n\r");
        else
          act_p("$C1 уже может проходить сквозь преграды.",
                 ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy( level / 4 );
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_PASS_DOOR;
    affect_to_char( victim, &af );

    act("$c1 становится полупрозрачн$gым|ым|ой.", victim, 0, 0, TO_ROOM);
    act("Ты становишься полупрозрачн$gым|ым|ой.", victim, 0, 0, TO_CHAR);
}

/*
 * 'nexus' spell
 */
SPELL_DECL(Nexus);
VOID_SPELL(Nexus)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *portal, *stone;
    Room *to_room = 0, *from_room;
    int vnum;

    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) 
        vnum = OBJ_VNUM_HOLY_PORTAL;
    else
        vnum = OBJ_VNUM_PORTAL;

    from_room = ch->in_room;

    if (victim->getModifyLevel() >= level + 3
      || !GateMovement( ch, victim ).canMove( ch )
      || saves_spell(ch->getModifyLevel(), victim, DAM_OTHER, ch, DAMF_SPELL)
      || IS_BLOODY( ch ))
    {
        ch->send_to("Твоя попытка закончилась неудачей.\n\r");
        return;
    }

    to_room = victim->in_room;
    stone = get_eq_char(ch,wear_hold);
    if (!ch->is_immortal() &&  (stone == 0 || stone->item_type != ITEM_WARP_STONE))
    {
        ch->send_to("Для этого заклинания требуется камень, искажающий пространство.\n\r");
        return;
    }

    if (stone != 0 && stone->item_type == ITEM_WARP_STONE)
    {
        ch->pecho( "Тебя пронизывает энергия, заключенная в %1$O6.\r\n"
                   "%1$^O1 ярко вспыхивает и исчезает!", stone );
        extract_obj(stone);
    }

    /* portal one */
    portal = create_object(get_obj_index(vnum),0);
    portal->timer = 1 + level / 5;
    portal->value[3] = to_room->vnum;

    obj_to_room(portal,from_room);

    act_p("Над землей образуется $o1.",ch,portal,0,TO_ROOM,POS_RESTING);
    act_p("Перед тобой образуется $o1.",ch,portal,0,TO_CHAR,POS_RESTING);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
        return;

    /* portal two */
    portal = create_object(get_obj_index(vnum),0);
    portal->timer = 1 + level / 5;
    portal->value[3] = from_room->vnum;

    obj_to_room(portal,to_room);

    if (to_room->people != 0)
    {
        act_p("Над землей образуется $o1.",
               to_room->people,portal,0,TO_ROOM,POS_RESTING);
        act_p("Над землей образуется $o1.",
               to_room->people,portal,0,TO_CHAR,POS_RESTING);
    }

}

/*
 * 'portal' spell
 */
SPELL_DECL(Portal);
VOID_SPELL(Portal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *portal, *stone;
    int vnum;
    
    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_DIVINE))
        vnum = OBJ_VNUM_HOLY_PORTAL;
    else
        vnum = OBJ_VNUM_PORTAL;

    if ( victim->getModifyLevel() >= level + 3
      || !GateMovement( ch, victim ).canMove( ch )
      || saves_spell(ch->getModifyLevel(), victim, DAM_OTHER, ch, DAMF_SPELL)
      || IS_BLOODY( ch ))
    {
        ch->send_to("Твоя попытка закончилась неудачей.\n\r");
        return;
    }


    stone = get_eq_char(ch,wear_hold);
    if (!ch->is_immortal()
    &&  (stone == 0 || stone->item_type != ITEM_WARP_STONE))
    {
        ch->send_to("Для этого заклинания требуется камень, искажающий пространство.\n\r");
        return;
    }

    if (stone != 0 && stone->item_type == ITEM_WARP_STONE)
    {
        ch->pecho( "Тебя пронизывает энергия, заключенная в %1$O6.\r\n"
                   "%1$^O1 ярко вспыхивает и исчезает!", stone );
             extract_obj(stone);
    }

    portal = create_object(get_obj_index(vnum),0);
    portal->timer = 2 + level / 8;
    portal->value[3] = victim->in_room->vnum;

    obj_to_room(portal,ch->in_room);

    act_p("Над землей образуется $o1.",ch,portal,0,TO_ROOM,POS_RESTING);
    act_p("Перед тобой образуется $o1.",ch,portal,0,TO_CHAR,POS_RESTING);

    if (victim->in_room->people != 0)
    {
        act_p("Странные токи пронизывают воздух.",
               victim->in_room->people,0,0,TO_ROOM,POS_RESTING);
        act_p("Странные токи пронизывают воздух.",
               victim->in_room->people,0,0,TO_CHAR,POS_RESTING);
    }


}

/*
 * 'solar flight' spell
 */
SPELL_DECL_T(SolarFlight, GateSpell);
VOID_SPELL(SolarFlight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (weather_info.sunlight != SUN_LIGHT) {
        ch->println( "Для этого тебе нужен солнечный свет." );
        return;
    }
                
    GateSpell::run( ch, victim, sn, level );
}



/*
 * 'gaseous form' spell
 */
SPELL_DECL(GaseousForm);
TYPE_SPELL(bool, GaseousForm)::checkPosition( Character *ch ) const
{
    if (!DefaultSpell::checkPosition( ch ))
        return false;

    if (!ch->fighting) {
        ch->println( "Ты можешь превратиться в туман только в бою!" );
        return false;
    }

    if (ch->mount) {
        ch->pecho( "Ты не можешь превратиться в туман, пока ты верхом или оседла%Gно|н|на!", ch );
        return false;
    }

    return true;
}
VOID_SPELL(GaseousForm)::run( Character *ch, Character *, int sn, int level ) 
{ 
    Room *target;

    if (ch->isAffected(sn)) {
        ch->println("Это заклинание использовалось совсем недавно.");
        return;
    }
    
    if (IS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)) {
        ch->println("Твоя попытка закончилась неудачей.");
        return;
    }

    target = get_random_room_vanish( ch );

    if (target && number_bits(1) == 1) {
        postaffect_to_char( ch, sn, 1 );
        
        transfer_char( ch, ch, target,
                    "%1$^C1 рассеивается в клубах тумана, принимая газообразную форму.\r\n%1$^C1 исчезает!",
                    "Ты рассеиваешься в клубах тумана, принимая газообразную форму.\r\nТы исчезаешь!",
                    "Неожиданно рядом с тобой начинает клубиться туман..\r\nИз сгустков тумана постепенно вырисовывается силуэт %1$C2.",
                    "Твое тело уплотняется, возвращаясь в обычное состояние." );
    } else {
        ch->send_to("Тебе не удалось превратиться в туман.\r\n");
    }
}


/*
 * 'teleport' spell
 */
SPELL_DECL(Teleport);
VOID_SPELL(Teleport)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (!ch->is_npc()) 
        victim = ch;

    if (HAS_SHADOW(victim) 
        || IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
        || (victim != ch && IS_SET(victim->imm_flags, IMM_SUMMON))
        || (!ch->is_npc() && victim->fighting)
        || (victim != ch && saves_spell( level - 5, victim, DAM_OTHER, ch, DAMF_SPELL)) 
        || IS_BLOODY(victim))
    {
        ch->send_to("Твоя попытка закончилась неудачей.\n\r");
        return;
    }
    
    victim->dismount( );
    transfer_char( victim, ch, get_random_room(victim),
                "%1$^C1 исчезает!",
                (victim != ch ? "Тебя телепортировали!" : ""),
                "%1$^C1 появляется из дымки." );
}



/*-----------------------------------------------------------------------
 * WordOfRecallMovement
 *-----------------------------------------------------------------------*/
class WordOfRecallMovement : public RecallMovement {
public:
    WordOfRecallMovement( Character *ch )
                     : RecallMovement( ch )
    {
    }
    WordOfRecallMovement( Character *ch, Character *actor, Room *to_room )
                     : RecallMovement( ch, actor, to_room )
    {
    }

protected:
    virtual bool findTargetRoom( )
    {
        int point;
        
        if (to_room)
            return true;

        if (ch->is_npc( ))
            return false;

        if (( point = ch->getClan( )->getRecallVnum( ) ) <= 0)
            point = ch->getPC( )->getHometown( )->getRecall( );

        if (point <= 0 || !( to_room = get_room_index( point ) )) {
            msgSelf( ch, "You are completely lost." );
            return false;
        }

        return true;
    }
    virtual bool canMove( Character *wch )
    {
        if (ch != actor)
            return checkForsaken( wch );
        else
            return checkMount( )
                   && checkBloody( wch )
                   && checkForsaken( wch );
    }
    virtual bool tryMove( Character *wch )
    {
        if (ch != actor)
            return applyInvis( wch );
        else
            return applyInvis( wch )
                   && applyMovepoints( );
    }
    virtual void movePet( NPCharacter *pet )
    {
        WordOfRecallMovement( pet, actor, to_room ).moveRecursive( );
    }
    virtual bool checkBloody( Character *wch )
    {
        if (IS_BLOODY(wch)) {
            msgSelfParty( wch, 
                          "...и никакого эффекта.", 
                          "Богам нет дела до %1$C2." );
            return false;
        }

        return true;
    }
    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
        if (fLeaving) 
             msgRoomNoParty( wch,
                             "%1$^C1 исчезает.",
                             "%1$^C1 и %2$C1 исчезают." );
        else
            msgRoomNoParty( wch, 
                            "%1$^C1 появляется в комнате.",
                            "%1$^C1 и %2$C1 появляются в комнате." );
    }
};

/*
 * 'word of recall' spell
 */
SPELL_DECL(WordOfRecall);
VOID_SPELL(WordOfRecall)::run( Character *ch, Character *, int sn, int level ) 
{ 
    if (ch->getProfession( ) == prof_samurai && ch->fighting) {
        ch->send_to("Твоя честь не позволяет тебе воспользоваться словом возврата!\n\r");
        return;
    }

    WordOfRecallMovement( ch ).move( );
}


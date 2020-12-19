/* $Id: group_beguiling.cpp,v 1.1.2.15.6.11 2009/08/16 02:50:31 rufina Exp $
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
 
#include "group_beguiling.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"

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

#include "merc.h"
#include "mercdb.h"
#include "occupations.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"


PROF(none);

#define OBJ_VNUM_MAGIC_JAR                93


SPELL_DECL(AttractOther);
VOID_SPELL(AttractOther)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if  ( ch->getSex( ) == victim->getSex( ) )
    {
        ch->send_to("Попробуй найти помощника более подходящего пола!\n\r");
        return;
    }
    spell(gsn_charm_person, level,ch,victim);
    return;
}

SPELL_DECL(CharmPerson);
VOID_SPELL(CharmPerson)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;

        if (is_safe(ch,victim) || overcharmed(ch) )
        {
                return;
        }

        if ( victim == ch )
        {
                ch->send_to("Ты нравишься себе еще больше!\n\r");
                return;
        }

        if ( !IS_AWAKE(victim) || !victim->can_see(ch) )
        {
                ch->send_to("Твоя жертва не видит тебя.\n\r");
                return;                
        }
        
        if ( (number_percent() > 50) && !victim->is_npc() )        
        {
                ch->pecho("Похоже, ты не так%1$Gое|ой|ая привлекательн%1$Gое|ый|ая, как тебе кажется.", ch );
                return;
        }
        

        if ( IS_CHARMED(victim)
                || IS_CHARMED(ch)
                || ( ch->getSex( ) == SEX_MALE &&  level < victim->getModifyLevel() )
                || ( ch->getSex( ) == SEX_FEMALE &&  level < ( victim->getModifyLevel()  ) )
                || IS_SET(victim->imm_flags,IMM_CHARM)
                || saves_spell( level, victim,DAM_CHARM, ch, DAMF_SPELL )
                || (victim->is_npc( ) 
                     && victim->getNPC( )->behavior 
                     && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
                || victim->is_immortal() )
        {
                ch->send_to("Не получилось...\n\r");
                return;
        }

        if ( victim->master )
                victim->stop_follower( );

        victim->add_follower( ch );

        victim->leader = ch;

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level         = ch->getRealLevel( );
        af.duration  = /*ch->isCoder() ? 1 : */number_fuzzy( level / 5 );
        
        af.bitvector.setValue(AFF_CHARM);
        affect_to_char( victim, &af );

        act_p( "$c1 очаровывает тебя!!!", ch, 0, victim, TO_VICT,POS_RESTING);

        if ( ch != victim )
                act_p("$C1 с обожанием смотрит на тебя.",ch,0,victim,TO_CHAR,POS_RESTING);

}

AFFECT_DECL(CharmPerson);
VOID_AFFECT(CharmPerson)::remove( Character *victim ) 
{
    DefaultAffectHandler::remove( victim );
    
    follower_clear(victim);

    if (victim->is_npc() 
        && victim->position == POS_SLEEPING
        && !IS_AFFECTED(victim, AFF_SLEEP))
        victim->position = victim->getNPC()->default_pos;
}
    


SPELL_DECL(ControlUndead);
VOID_SPELL(ControlUndead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        

        if  ( !victim->is_npc() || !IS_SET(victim->act,ACT_UNDEAD) )
        {
                act_p("$C1 не похо$gже|ж|жа на живого мертвеца.",ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }
        spell(gsn_charm_person,level,ch,victim);
        return;

}



SPELL_DECL(LovePotion);
VOID_SPELL(LovePotion)::run( Character *ch, Character *, int sn, int level ) 
{ 
    if (ch->isAffected(sn)) {
        ch->pecho("Ты уже настолько неотразим%Gо||а, насколько это возможно!", ch);
        return;
    }

    Affect af;

    af.bitvector.setTable(&affect_flags);
    af.type               = sn;
    af.level              = level;
    af.duration           = level / 5;
    affect_join(ch, &af);

    ch->pecho("Ты чувствуешь себя неотразимо прекрасн%Gым|ым|ой.", ch);
}

AFFECT_DECL(LovePotion);
VOID_AFFECT(LovePotion)::look( Character *ch, Character *witch, Affect *paf ) 
{
    Affect af;

    DefaultAffectHandler::look( ch, witch, paf );
    
    if (ch == witch || ch->is_immortal())
        return;

    if (is_safe(witch, ch) || overcharmed(witch))
        return;

    if (saves_spell( paf->level, ch, DAM_CHARM, witch, DAMF_SPELL )) {
        act("При взгляде на $c4 твое сердце на мгновение замирает.", witch, 0, ch, TO_VICT);
        act("Во взгляде $C2 на мгновение мелькает полный восторг.", witch, 0, ch, TO_CHAR);
        return;
    }

    act("Неужели $c1 выглядит так очаровательно?", witch, 0, ch, TO_VICT);
    act("$C1 смотрит на тебя с покорностью.", witch, 0, ch, TO_CHAR);
    act("$C1 зачарованно смотрит на $c4 и жаждет выполнить любые поручения.", witch, 0, ch, TO_NOTVICT);

    if (ch->master)
        ch->stop_follower( );

    ch->add_follower( witch );
    ch->leader = witch;

    af.bitvector.setTable(&affect_flags);
    af.type      = gsn_charm_person;
    af.level     = ch->getModifyLevel( );
    af.duration  = number_fuzzy( witch->getModifyLevel( ) / 4);
    af.bitvector.setValue(AFF_CHARM);
    affect_to_char(ch, &af);
}

/*
 * magic jar behavior
 */
void MagicJar::get( Character *ch )
{
    if (!ch->is_npc( ) && strstr(obj->getName( ), ch->getNameP( )) != 0) {
        act("Вот это удача!",ch,obj,0,TO_CHAR);
        extract_obj(obj);
    }
    else
        act("Ты заполучи%gло|л|ла блудную душу.",ch,obj,0,TO_CHAR);
} 

bool MagicJar::extract( bool fCount )
{
    Character *wch;

    for (wch = char_list; wch != 0 ; wch = wch->next)
    {
        if (wch->is_npc()) 
            continue;

        if (strstr(obj->getName( ),wch->getNameP( )) != 0)
        {
            if (IS_SET( wch->act, PLR_NO_EXP )) {
                REMOVE_BIT(wch->act,PLR_NO_EXP);
                wch->send_to("Твоя душа возвращается к тебе.\n\r");
            }

            break;
        }
    }

    return ObjectBehavior::extract( fCount );
}

bool MagicJar::quit( Character *ch, bool count )
{
    extract_obj( obj );
    return true;
}

bool MagicJar::area( )
{
    Character *carrier = obj->carried_by;
    
    if (!carrier
        || carrier->is_npc( )
        || IS_SET(carrier->in_room->room_flags, 
                  ROOM_SAFE|ROOM_SOLITARY|ROOM_PRIVATE)
        || !carrier->in_room->pIndexData->guilds.empty( ))
    {
        extract_obj( obj );
        return true;
    }
    else
        return false;
}
    
SPELL_DECL(MagicJar);
VOID_SPELL(MagicJar)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Object *vial;
    Object *jar;
    char buf[MAX_STRING_LENGTH];

    if (victim == ch)
        {
        ch->send_to("Твоя душа всегда с тобой!\n\r");
        return;
        }

    if (victim->is_npc())
        {
        ch->send_to("Душа этого противника неподвластна тебе!.\n\r");
        return;
        }

    if ( IS_SET( victim->act, PLR_NO_EXP ) )
    {
        ch->send_to("Душа твоего противника где-то далеко...\n\r");
        return;
    }


    if (saves_spell(level ,victim,DAM_MENTAL, ch, DAMF_SPELL))
       {
        ch->send_to("Твоя попытка закончилась неудачей.\n\r");
        return;
       }

    for( vial=ch->carrying; vial != 0; vial=vial->next_content )
        if ( vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL )
            break;

    if (  vial == 0 )  {
        ch->send_to("У тебя нет пустого сосуда, чтоб заточить в него дух противника.\n\r");
        return;
    }
    
    extract_obj(vial);

    jar        = create_object(get_obj_index(OBJ_VNUM_MAGIC_JAR), 0);
    jar->setOwner(ch->getNameP( ));
    jar->from = str_dup(ch->getNameP( ));
    jar->level = ch->getRealLevel( );

    jar->fmtName( jar->getName( ), victim->getNameP( ));
    jar->fmtShortDescr( jar->getShortDescr( ), victim->getNameP( ));
    jar->fmtDescription( jar->getDescription( ), victim->getNameP( ));

    sprintf( buf,jar->pIndexData->extra_descr->description, victim->getNameP( ) );
    jar->extra_descr = new_extra_descr();
    jar->extra_descr->keyword = str_dup( jar->pIndexData->extra_descr->keyword );
    jar->extra_descr->description = str_dup( buf );
    jar->extra_descr->next = 0;

    jar->level = ch->getRealLevel( );
    jar->cost = 0;
    obj_to_char( jar , ch );

    SET_BIT(victim->act,PLR_NO_EXP);
    act("Дух $C2 теперь заточен в сосуде и находится в твоей власти.", ch, 0, victim, TO_CHAR);
    act("$c1 {Rзаточил твой дух в сосуде.{x", ch, 0, victim, TO_VICT);
}

SPELL_DECL(MysteriousDream);
VOID_SPELL(MysteriousDream)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Affect af;

  if (IS_SET(room->room_flags, ROOM_LAW))
    {
      ch->send_to("Божественные Силы покровительствуют этому месту.\n\r");
      return;
    }
    if ( room->isAffected( sn ))
    {
     ch->send_to("Эта комната уже наполнена усыпляющим газом.\n\r");
     return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    
    af.modifier  = 0;
    af.bitvector.setValue(AFF_ROOM_SLEEP);
    room->affectTo( &af );

    ch->send_to("Комната превращается в самое уютное место для сна.\n\r");
    act_p("Комната превращается в самое прекрасное место для твоего отдыха.\n\r",
           ch,0,0,TO_ROOM,POS_RESTING);


}

AFFECT_DECL(MysteriousDream);
VOID_AFFECT(MysteriousDream)::entry( Room *room, Character *ch, Affect *paf )
{
     act("{yВ воздухе клубится сонный туман.{x",ch, 0, 0, TO_CHAR);
}

VOID_AFFECT(MysteriousDream)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Сонный туман, клубящийся в воздухе, развеется через {W%1$d{x ча%1$Iс|са|сов.",
                   paf->duration )
        << endl;
}

VOID_AFFECT(MysteriousDream)::update( Room *room, Affect *paf )
{
    Affect af;
    Character *vch;

    af.bitvector.setTable(&affect_flags);
    af.type         = gsn_sleep;
    af.level         = paf->level - 1;
    af.duration        = number_range(1,((af.level/10)+1));
    
    af.modifier        = -5;
    af.bitvector.setValue(AFF_SLEEP);

    for (vch = room->people; vch != 0; vch = vch->next_in_room) {
        if ( !saves_spell(af.level - 4,vch,DAM_CHARM, 0, DAMF_SPELL)
                && !is_safe_rspell(paf->level,vch)
                && !(vch->is_npc() && IS_SET(vch->act,ACT_UNDEAD) )
                && !IS_AFFECTED(vch,AFF_SLEEP) && number_bits(3) == 0 )
        {
            if ( IS_AWAKE(vch) )
            {
                vch->send_to("Ты засыпаешь...\n\r");
                act_p("$c1 засыпает.",vch,0,0,TO_ROOM,POS_RESTING);
                vch->position = POS_SLEEPING;
            }

            affect_join(vch,&af);
        }
    }
}


SPELL_DECL(Sleep);
VOID_SPELL(Sleep)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_SLEEP)
    ||   (victim->is_npc() && IS_SET(victim->act,ACT_UNDEAD))
    ||   level < victim->getModifyLevel()
    ||   saves_spell( level-4, victim,DAM_CHARM, ch, DAMF_SPELL ) )
    {
        ch->println("Не получилось...");
            return;
    }

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = 1 + level/10;
    
    af.bitvector.setValue(AFF_SLEEP);
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
        act("Ты чувствуешь себя очень сонн$gым|ым|ой.... ты засыпаешь..", victim, 0, 0, TO_CHAR);
        act("$c1 засыпает.", victim, 0, 0, TO_ROOM);
        victim->position = POS_SLEEPING;
    }
    return;

}

AFFECT_DECL(Sleep);
VOID_AFFECT(Sleep)::remove( Character *victim ) 
{
    DefaultAffectHandler::remove( victim );

    if(victim->is_npc() && victim->position == POS_SLEEPING)
        victim->position = victim->getNPC()->default_pos;
}


SPELL_DECL(Terangreal);
VOID_SPELL(Terangreal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if (victim->is_npc())
        return;

    af.bitvector.setTable(&affect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = 10;
    af.bitvector.setValue(AFF_SLEEP);
    affect_join( victim, &af );

    if ( IS_AWAKE(victim) )
    {
        victim->send_to("Внезапная волна сонливости накатывает на тебя.\n\r");
        act_p( "$c1 забывается глубоким сном.",
                victim, 0, 0, TO_ROOM,POS_RESTING);
        victim->position = POS_SLEEPING;
    }

    return;

}


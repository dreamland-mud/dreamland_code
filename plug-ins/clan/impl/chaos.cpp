/* $Id: chaos.cpp,v 1.1.6.6.4.14 2010-09-01 21:20:44 rufina Exp $
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

#include "chaos.h"

#include "affecthandlertemplate.h"
#include "spelltemplate.h"                                                 
#include "skill.h"
#include "skillmanager.h"
#include "skillcommandtemplate.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "act.h"
#include "act_move.h"
#include "interp.h"
#include "weapongenerator.h"
#include "gsn_plugin.h"
#include "handler.h"
#include "merc.h"
#include "vnum.h"
#include "mercdb.h"
#include "fight.h"
#include "magic.h"
#include "def.h"

#define MOB_VNUM_MIRROR_IMAGE       17
#define OBJ_VNUM_CHAOS_BLADE        87

GSN(dispel_affects);

/*--------------------------------------------------------------------------
 * Ivan 
 *-------------------------------------------------------------------------*/
void ClanGuardChaos::actGreet( PCharacter *wch )
{
    interpret_raw( ch, "say", "Растворись в Хаосе, %s!", wch->getNameP( '1' ).c_str( ) );
}
void ClanGuardChaos::actPush( PCharacter *wch )
{
    act( "На мгновенье ты теряешь представление о реальности...", wch, 0, ch, TO_CHAR );
    act( "$C1 выпускает частицу ХАОСА в $c2\n\r...и $c1 растворяется в нем...", wch, 0, ch, TO_ROOM );
}
int ClanGuardChaos::getCast( Character *victim )
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
                sn = gsn_acid_arrow;
                break;
        case  4: 
        case  5:
                sn = gsn_caustic_font;
                break; 
        case  6:
        case  7:
        case  8:
                sn = gsn_acid_blast;
                break;
        case  9:
                sn = gsn_disgrace;
                break;
        case 10:
                if ( ch->hit < (ch->max_hit / 3) )
                        sn = gsn_garble;
                break;
        }

        return sn;
}


/*
 * chaos blade behavior
 */
void ChaosBlade::fight( Character *ch )
{
    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
        return;

    switch(number_bits(6)) {
    case 0:
            act("{MКлинок Хаоса{x неистово вздрагивает!",ch,0,0,TO_ROOM);
            ch->send_to("Твой {MКлинок Хаоса{x неистово вздрагивает!\n\r");
            spell(gsn_mirror, ch->getModifyLevel(),ch,ch, FSPELL_BANE );
            ch->setWaitViolence( 2 );
            break;

    case 1:
            act("{MКлинок Хаоса{x устрашающе звенит!",ch,0,0,TO_ROOM);
            ch->send_to("Твой {MКлинок Хаоса{x устрашающе звенит!\n\r");
            spell(gsn_garble, ch->getModifyLevel()+5, ch, ch->fighting, FSPELL_BANE );
            ch->setWaitViolence( 2 );
            break;

    case 2:
            act("{MКлинок Хаоса{x неудержимо дрожит!",ch,0,0,TO_ROOM);
            ch->send_to("Твой {MКлинок Хаоса{x неудержимо дрожит!\n\r");
            spell(gsn_confuse, ch->getModifyLevel()+5,ch,ch->fighting, FSPELL_BANE );
            ch->setWaitViolence( 2 );
            break;
    }
}

bool ChaosBlade::death( Character *ch )
{
    act_p( "Твой {MКлинок Хаоса{x исчезает.", ch, 0, 0, TO_CHAR, POS_DEAD );
    act( "{MКлинок Хаоса{x $c2 исчезает.", ch, 0, 0, TO_ROOM );
    extract_obj( obj );
    return false;
}

SPELL_DECL(ChaosBlade);
VOID_SPELL(ChaosBlade)::run(Character *ch, char *, int sn, int level)
{
    Object *blade;

    blade = create_object(get_obj_index(OBJ_VNUM_CHAOS_BLADE), level);
    ch->send_to("Ты взмахиваешь руками и создаешь Клинок Хаоса!\n\r");
    act_p("$c1 взмахивает руками и создает Клинок Хаоса!",
          ch, 0, 0, TO_ROOM, POS_RESTING);

    blade->timer = level * 2;
    blade->level = ch->getModifyLevel();

    WeaponGenerator()
        .item(blade)
        .skill(sn)
        .valueTier(number_range(2, 3))
        .hitrollTier(number_range(2, 3))
        .damrollTier(number_range(2, 3))
        .assignValues()
        .assignHitroll()
        .assignDamroll();

    obj_to_char(blade, ch);
}

SPELL_DECL(Confuse);
VOID_SPELL(Confuse)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;
        Character *rch;
        int count=0;

        if ( victim->isAffected(gsn_confuse) )
        {
                act_p("Кто-то совсем недавно уже ввел в заблуждение $C4.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }

        if ( saves_spell(level,victim, DAM_MENTAL, ch, DAMF_MAGIC) )
        {
                ch->send_to("Не получилось.\n\r");
                return;
        }

        victim->send_to("Тебя сконфузили!\n\r");
        act("$c1 выглядит очень сконфуженно.",victim,0,0,TO_ROOM);

        af.type      = sn;
        af.level     = level;
        af.duration  = level / 50;
        affect_to_char(victim,&af);

        for ( rch = ch->in_room->people; rch != 0; rch = rch->next_in_room )
        {
                if ( rch == ch
                        && !ch->can_see( rch )
                        && ch->get_trust() < rch->invis_level )
                {
                        count++;
                }

                continue;
        }

        for ( rch = ch->in_room->people; rch != 0; rch = rch->next_in_room )
        {
                if ( rch != ch
                        && ch->can_see( rch )
                        && ch->get_trust() >= rch->invis_level
                        && number_range(1,count) == 1 )
                        break;
        }

        if ( rch )
            interpret_raw( victim, "murder", rch->getNameP( ));

        interpret_raw( victim, "murder", ch->getNameP( ));

}




SPELL_DECL(Disgrace);
VOID_SPELL(Disgrace)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;

  if (!victim->isAffected(sn) && !saves_spell(level, victim, DAM_MENTAL, ch, DAMF_MAGIC))
    {
      af.type               = sn;
      af.level              = level;
      af.duration           = level;
      af.location = APPLY_CHA;
      af.modifier           = ( - ( 5 + level / 10 ) );
      affect_to_char(victim,&af);
      
      act("$c1 выглядит гораздо менее уверенн$gым|ым|ой в себе!", victim, 0, 0, TO_ROOM);
      act("Ты чувствуешь себя гораздо менее уверенно!", victim, 0, 0, TO_CHAR);
    }
      else
      ch->send_to("Твоя попытка закончилась неудачей.\n\r");

}




SPELL_DECL(Disperse);
VOID_SPELL(Disperse)::run( Character *ch, Room *room, int sn, int level ) 
{ 

    Room *pRoomIndex;
    int cnt = 0;

    if ( ch->isAffected(sn ) )
    {
        ch->send_to("У тебя не хватает сил разогнать эту группу.\n\r");
        return;
    }

    if (IS_SET(room->room_flags, ROOM_NO_RECALL)) {
        ch->println("Отсюда ты никого не сможешь вышвырнуть.");
        return;
    }

    for ( auto &vch : room->getPeople() )
    {
        
        if (vch == ch)
            continue;
            
        if (vch->is_immortal())
            continue;

        if (IS_SET(vch->imm_flags, IMM_SUMMON))
            continue;
        
        if (vch->is_npc( )) {
            if (IS_SET(vch->act, ACT_AGGRESSIVE))
                continue;

            if (vch->leader 
                && !vch->leader->is_npc( )
                && is_safe_nomessage( ch, vch->leader ))
                continue;
        }
        else {
            if (vch->getModifyLevel() <= PK_MIN_LEVEL)
                continue;

            if (is_safe_nomessage( ch, vch ))
                continue;
        }
        
        for ( ; ; ) {
            pRoomIndex = get_random_room( vch );

            if (!IS_SET(pRoomIndex->room_flags, ROOM_NO_RECALL) )
                    break;
        }
        
        cnt++;
        vch->dismount( );
        transfer_char( vch, ch, pRoomIndex,
                "%1$^C1 исчезает!",
                "Мир закружился вокруг тебя подчиняясь Силам Хаоса!",
                "%1$^C1 с озадаченным видом появляется рядом с тобой." );
    }
    
    postaffect_to_char( ch, sn, 15 );
    
    if (cnt == 0)
        ch->println("Подходящей жертвы не нашлось.");
}




SPELL_DECL(Doppelganger);
VOID_SPELL(Doppelganger)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect af;

  if (ch->is_npc() || victim->is_npc())
    {
     act("Ты не можешь подражать $C3.",ch,0,victim,TO_CHAR);
     return;
   }

  if (ch == victim || ch->getDoppel( ) == victim)
    {
      act("Ты уже выглядишь как $E.",ch,0,victim,TO_CHAR);
      return;
    }

  if (victim->is_immortal() || victim->getDoppel( ) == ch)
    {
      ch->send_to("Ну..Ну...\n\r");
      return;
    }

  if (saves_spell(level,victim, DAM_CHARM, ch, DAMF_MAGIC))
   {
    ch->send_to("Твоя попытка закончилась неудачей.\n\r");
    return;
   }

  act("Ты меняешь свой облик, подражая $C3.", ch,0,victim,TO_CHAR);
  act("$c1 меняет свой облик, подражая ТЕБЕ!", ch,0,victim,TO_VICT);
  act("$c1 меняет свой облик, подражая $C3!", ch,0,victim,TO_NOTVICT);

  af.type               = sn;
  af.level              = level;
  af.duration           = (2 * level)/3;  
  affect_to_char(ch,&af);
  ch->doppel = victim;


}




SPELL_DECL(Garble);
VOID_SPELL(Garble)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  if( ch == victim ) {
    ch->send_to("Это просто глупо.\n\r");
    return;
  }

  if( victim->isAffected(sn ) )  {
    act_p( "$C1 и так уже не может ничего внятно произнести.",
            ch, 0, victim, TO_CHAR,POS_RESTING);
    return;
  }

  if( is_safe_nomessage( ch, victim ) ) {
    ch->send_to("Тебе не дано здесь чего-нибудь добиться.\n\r");
    return;
  }

  if( saves_spell( level, victim, DAM_MENTAL, ch, DAMF_MAGIC ) ) {
    ch->send_to("Хаос остался в тебе.\n\r");
    return;
  }

    gsn_garble->getCommand()->run(victim, level);

  act_p( "Ты поделил$gось|ся|ась частицей хаоса с языком $C2.",
          ch, 0, victim, TO_CHAR,POS_RESTING);
  victim->send_to("Такое впечатление, что твой язык завязали узлом.\n\r");

}

SKILL_DECL(garble);
BOOL_SKILL(garble)::run( Character *victim, int slevel ) 
{
    Affect af;

    af.type      = gsn_garble;
    af.level     = slevel;
    af.duration  = 1;
    affect_to_char( victim, &af );
    return true;
}




SPELL_DECL(Mirror);
VOID_SPELL(Mirror)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;
        int mirrors,new_mirrors;
        Character *tmp_vict, *gch;
        char long_buf[MAX_STRING_LENGTH];
        char short_buf[20];

        if (victim->is_npc())
        {
                ch->send_to("Зеркальные отражения могут иметь только персонажи.\n\r");
                return;
        }

        if ( ch->isAffected(sn ) )
        {
            ch->send_to("Ты пытаешься сотворить зеркальные отражения, но безуспешно.\n\r");
            return;
        }

        for (mirrors = 0, gch = char_list; gch != 0; gch = gch->next)
                if (gch->is_mirror() && gch->doppel == victim)
                        mirrors++;

        if ( ( mirrors >= level/5 ) || ( mirrors >= 10 ) )
        {
                if (ch==victim)
                        ch->send_to("Зеркальных отражений уже слишком много.\n\r");
                else
                        act_p("Зеркальных отражений $C2 уже слишком много.",
                                ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }

        postaffect_to_char(ch, sn, 2);

        af.level     = level;
        
        tmp_vict = victim->getDoppel( );

        sprintf(long_buf, "{W%s{x%s здесь.\n\r",
                tmp_vict->getNameP( '1' ).c_str(), tmp_vict->getPC( )->getParsedTitle( ).c_str( ));
        strcpy(short_buf, tmp_vict->getNameP( ) );

        if ( ch == victim )
        {
                ch->send_to("Около тебя появляется твое зеркальное отражение!\n\r");
                act_p("Зеркальное отражение $c2 появляется рядом с $y!",
                        ch,0,victim,TO_ROOM,POS_RESTING);
        }
        else
        {
                act_p("Зеркальное отражение $C2 появляется рядом с $Y!",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("Зеркальное отражение $C2 появляется рядом с $Y!",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                victim->send_to("Рядом с тобой появляется твое зеркальное отражение!\n\r");
        }

        for (new_mirrors=0;
                        (mirrors + new_mirrors < level/5) && (mirrors + new_mirrors < 10);
                        new_mirrors++)
        {   
                NPCharacter *mch;

                mch = create_mobile( get_mob_index(MOB_VNUM_MIRROR_IMAGE) );
                SET_BIT(mch->affected_by ,AFF_SNEAK);
                mch->setShortDescr( short_buf );
                mch->setLongDescr( long_buf );
                mch->setDescription( tmp_vict->getDescription( ) );
                
                DLString name( tmp_vict->getNameP( ) );
                mch->setName( name );
                
                mch->setSex( tmp_vict->getSex( ) );

                af.type = gsn_doppelganger;
                af.duration = level;
                affect_to_char(mch,&af);

                af.type = gsn_mirror;
                af.duration = -1;
                affect_to_char(mch,&af);

                mch->max_hit = mch->hit = 1;
                mch->setLevel( 1 );
                mch->doppel = victim;
                mch->master = victim;
                char_to_room(mch,victim->in_room);
        }

}




SPELL_DECL(Randomizer);
VOID_SPELL(Randomizer)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;

    if ( ch->isAffected(sn ) )
    {
      ch->send_to("Это заклинание использовалось совсем недавно.\n\r");
      return;
    }

//  if (IS_SET(room->room_flags, ROOM_LAW))
//    {
//      ch->send_to("Здесь тебе противостоят Силы Порядка.\n\r");
//      return;
//    }
    if (IS_ROOM_AFFECTED(room, AFF_ROOM_RANDOMIZER))
    {
        ch->send_to("Магические Силы Хаоса уже изменили окружающий мир.\n\r");
        return;
    }

  if (number_bits(1) == 0)
    {
      ch->send_to("Несмотря на твои усилия, окружающий мир противостоит Хаосу.\n\r");
      postaffect_to_char(ch, sn, level / 10);
      return;
    }

    af.bitvector.setTable(&raffect_flags);
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 15;
    af.bitvector.setValue(AFF_ROOM_RANDOMIZER);
    room->affectTo( &af );

    postaffect_to_char(ch, sn, level / 5);

    ch->send_to("Окружающее тебя пространство теперь находится под властью Хаоса!\n\r");
    ch->send_to("Использование Магических Сил Хаоса опустошает тебя.\n\r");
    ch->hit -= min(200, ch->hit/2);
    act_p("Магические Силы Хаоса изменяют окружающий мир.",
           ch,0,0,TO_ROOM,POS_RESTING);
}


AFFECT_DECL(Randomizer);
VOID_AFFECT(Randomizer)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Окружающее тебя пространство на ближайш%1$Iий|ие|ие {W%1$d{x "
                   "ча%1$Iс|са|сов попало под власть Хаоса!", paf->duration ) 
        << endl;
}

AFFECT_DECL(Doppelganger);
VOID_AFFECT(Doppelganger)::remove( Character *victim ) 
{
    DefaultAffectHandler::remove( victim );                                     

    if (victim->is_mirror( ))
        victim->stop_follower( );

    victim->doppel = NULL;
}


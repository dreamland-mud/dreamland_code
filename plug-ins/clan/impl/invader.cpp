/* $Id: invader.cpp,v 1.1.6.7.6.20 2010-09-01 21:20:44 rufina Exp $
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

#include "invader.h"
#include "clanorg.h"

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "act.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "mercdb.h"
#include "fight.h"
#include "handler.h"
#include "vnum.h"
#include "clanreference.h"
#include "magic.h"
#include "def.h"

GSN(dispel_affects);
GSN(soul_lust);
GSN(shadow_shroud);
CLAN(invader);

/*--------------------------------------------------------------------------
 * Neere 
 *-------------------------------------------------------------------------*/
void ClanGuardInvader::actGreet( PCharacter *wch )
{
    do_say(ch, "Приветствую тебя, идущий темным путем.");
}
void ClanGuardInvader::actPush( PCharacter *wch )
{
    act( "$C1 зверски ухмыляется тебе...\n\rТы теряешь рассудок от страха и куда-то несешься.", wch, 0, ch, TO_CHAR );
    act( "$C1 сверлит глазами $c4 и $c1 с испугу куда-то уносится.", wch, 0, ch, TO_ROOM );
}
int ClanGuardInvader::getCast( Character *victim )
{
        int sn = -1;

        switch ( dice(1,16) )
        {
        case  0:
        case  1:
                sn = gsn_blindness;
                break;
        case  2:
        case  3:
                if (!victim->isAffected( gsn_spellbane ))
                    sn = gsn_dispel_affects;
                break;
        case  4:
        case  5:
                sn = gsn_weaken;
                break;
        case  6: 
        case  7:
                sn = gsn_energy_drain;
                break;
        case  8: 
        case  9:
                sn = gsn_plague;
                break;
        case 10:
        case 11:
                sn = gsn_acid_arrow;
                break;
        case 12:
        case 13:
        case 14:
                sn = gsn_acid_blast;
                break;
        case 15:
                if ( ch->hit < (ch->max_hit / 3) )
                        sn = gsn_shadow_cloak;
                else
                        sn = -1;
                break;
        default:
                sn = -1;
                break;
        }

        return sn;
}

/*
 * 'fade' skill command
 */

SKILL_RUNP( fade )
{
        if ( !gsn_fade->available( ch ))
        {
            ch->send_to("Ась?\n\r");
            return;
        }

        if ( MOUNTED(ch) )
        {
                ch->send_to("Ты не можешь исчезнуть, когда в седле.\n\r");
                return;
        }

        if ( RIDDEN(ch) )
        {
                ch->send_to("Ты не можешь исчезнуть, когда ты оседлан.\n\r");
                return;
        }

        if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE) )
        {
                ch->send_to("Ты не можешь скрыться, когда светишься.\n\r");
                return;
        }

        if ( !gsn_fade->usable( ch ) )
                return;

        ch->send_to("Ты пытаешься исчезнуть.\n\r");

        int k = ch->getLastFightDelay( );

        if ( k >= 0 && k < FIGHT_DELAY_TIME )
                k = k * 100 /        FIGHT_DELAY_TIME;
        else
                k = 100;

        if ( number_percent() < gsn_fade->getEffective( ch ) * k / 100 )
        {
                SET_BIT(ch->affected_by, AFF_FADE);
                gsn_fade->improve( ch, true );
        }
        else
                gsn_fade->improve( ch, false );

        return;
}



SPELL_DECL(EvilSpirit);
VOID_SPELL(EvilSpirit)::run( Character *ch, Room *room, int sn, int level ) 
{ 
 AREA_DATA *pArea = room->area;
 Affect af,af2;

 if (IS_RAFFECTED(room, AFF_ROOM_ESPIRIT)
        || room->isAffected( sn) )
  {
   ch->send_to("Эта зона полностью под контролем злых духов.\n\r");
   return;
  }

 if ( ch->isAffected(sn ) )
    {
      ch->send_to("У тебя недостаточно энергии.\n\r");
      return;
    }

  if (IS_SET(room->room_flags, ROOM_LAW)
        || IS_SET(room->area->area_flag,AREA_HOMETOWN) )
    {
      ch->send_to("Святая аура в этой комнате не дает творить зло.\n\r");
      return;
    }

    af2.where     = TO_AFFECTS;
    af2.type      = sn;
    af2.level          = ch->getModifyLevel();
    af2.duration  = level / 5;
    af2.modifier  = 0;
    af2.location  = APPLY_NONE;
    af2.bitvector = 0;
    affect_to_char( ch, &af2 );

    af.where     = TO_ROOM_AFFECTS;
    af.type      = sn;
    af.level     = ch->getModifyLevel();
    af.duration  = level / 25;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_ROOM_ESPIRIT;

    for (map<int, Room *>::iterator i = pArea->rooms.begin( ); i != pArea->rooms.end( ); i++)
    {
        room = i->second;
        room->affectTo( &af );
        act("Частица первородного зла проникает в этот мир.", room->people,0,0,TO_ALL);
    }
}

SPELL_DECL(EyesOfIntrigue);
VOID_SPELL(EyesOfIntrigue)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
        Character *victim;

        if (( victim = get_char_world_doppel( ch, target_name ) ) == 0 || DIGGED(victim))
        {
                ch->send_to("Тьма не может обнаружить такого персонажа.\n\r");
                return;
        }

        if(is_safe_nomessage(ch,victim) 
            || (victim->is_npc( ) && IS_SET(victim->act, ACT_NOEYE)))
        {
                ch->send_to("Извини, тьма тут бессильна.\n\r");
                return;
        }

        if( victim->isAffected(gsn_golden_aura ) )
        {
                if(saves_spell(level, victim, DAM_OTHER, ch, DAMF_SPELL))
                {
                        ch->send_to("У тебя не хватает силы приказать темноте.\n\r");
                        return;
                }

                victim->pecho( "На миг тебя окружает темнота, из которой на тебя смотрит огромный глаз.\n\r"
                               "...И тихий звон {Wауры{x рождает имя - {D%#^C1{x.", ch );
        }
        
        do_look_auto( ch, victim->in_room );
}




SPELL_DECL(Nightfall);
VOID_SPELL(Nightfall)::run( Character *ch, Room *room, int sn, int level ) 
{ 
  Character *vch;
  Object  *light;
  Affect af;

  if( ch->isAffected(sn ) ) {
    ch->send_to("У тебя не достаточно энергии для контроля над светом.\n\r");
    return;
  }

        if (IS_SET(room->room_flags,ROOM_SAFE))
        {
                ch->send_to("Тут нельзя даже такую мелочь, как эта.\n\r");
                return;
        }

  for( vch = room->people; vch != 0; vch = vch->next_in_room )
    for( light = vch->carrying; light != 0; light = light->next_content )
      if( light->item_type == ITEM_LIGHT && !is_same_group( ch, vch ) ) {
        damage_to_obj( vch, light, light, light->condition / 2 + number_range( 1, light->condition ) );
          }

  for( light = room->contents;light != 0; light=light->next_content )
    if (light->item_type == ITEM_LIGHT ) {
      damage_to_obj( vch, light, light, light->condition / 2 + number_range( 1, light->condition ) );
    }

    af.where         = TO_AFFECTS;
    af.type      = sn;
    af.level         = level;
    af.duration  = 2;
    af.modifier  = 0;
    af.location  = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char( ch, &af );

}

SPELL_DECL_T(Nightwalker, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, Nightwalker)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
                         (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit), 
                         ch->max_mana,
                         number_range(level/15, level/10),
                         number_range(level/3, level/2),
                         0 );
}


SPELL_DECL(ShadowCloak);
VOID_SPELL(ShadowCloak)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;
    DLString msgChar, msgVict, orgCh, orgVict;
    
    if (ch->is_npc( ) || victim->is_npc( ) || ch->getClan( ) != victim->getClan( )) {
        ch->println("Это заклинание ты можешь произнести только на члена твоего клана.");
        return;
    }
    
    orgCh = ClanOrgs::getAttr( ch->getPC( ) );
    orgVict = ClanOrgs::getAttr( victim->getPC( ) );

    if (orgCh != orgVict) {
        ch->println("Это заклинание ты можешь произнести только на члена твоей организации.");
        return;
    }

    if (victim->isAffected( gsn_soul_lust )) {
        ch->pecho( ch == victim ? 
                          "Жажда душ уже горит в тебе." :
                          "Жажда душ уже горит в %C6.", victim );
        return;
    }

    if (victim->isAffected( sn ) || victim->isAffected( gsn_shadow_shroud )) {
        ch->pecho( ch == victim ? 
                          "Призрачная мантия уже защищает тебя." : 
                          "Призрачная мантия уже защищает %C4.", victim );
        return;
    }

    if (orgCh == "killers") {
        msgVict = "Призрачная мантия окутывает тебя. Ты погружаешься во тьму.";
        msgChar = "%2$C1 окутывается тьмой.";
        sn = gsn_shadow_shroud;
    }
    else if (orgVict == "adepts") {
        msgVict = "В тебе загорается огонь, жаждущий душ ангелов.";
        msgChar = "В %2$C6 загорается огонь, жаждущий душ ангелов.";
        sn = gsn_soul_lust;
    }
    else {
        msgVict = "Призрачная мантия окутывает тебя.";
        msgChar = "Призрачная мантия окутывает %2$C4.";
    }

    af.type      = sn;
    af.level         = level;
    af.duration  = 24;

    af.where     = TO_DETECTS;
    af.bitvector = DETECT_GOOD | DETECT_FADE;
    af.location  = APPLY_SAVING_SPELL;
    af.modifier  = ch->applyCurse( 0 - level / 9 );
    affect_to_char( victim, &af );

    af.where     = TO_AFFECTS;
    af.bitvector = IS_AFFECTED(victim, AFF_PROTECT_GOOD) ? 0 : AFF_PROTECT_GOOD;
    af.modifier  = ch->applyCurse( -level * 5 / 2 );
    af.location  = APPLY_AC;
    affect_to_char( victim, &af );
    
    victim->pecho( msgVict.c_str( ), ch, victim );
    if (ch != victim)
        ch->pecho( msgChar.c_str( ), ch, victim );
}




SPELL_DECL(Shadowlife);
VOID_SPELL(Shadowlife)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        Affect af;

        if (victim->is_npc())
        {
                ch->send_to("Бесполезная трата сил и энергии...\n\r");
                return;
        }

        if (ch->isAffected(sn))
        {
                ch->send_to("У тебя недостаточно энергии, чтобы создать тень.\n\r");
                return;
        }

        if ( victim->isAffected(gsn_golden_aura )
                && saves_spell(level, victim, DAM_OTHER, ch, DAMF_SPELL) )
        {
                ch->send_to("Тьма пытается сделать это для тебя, но не может.\n\r");
                victim->send_to("Около тебя появляется твоя тень, но тут же исчезает.\n\r");
                return;
        }

  act_p("Ты даешь жизнь тени $C2!",ch, 0, victim, TO_CHAR,POS_RESTING);
  act_p("$c1 дает жизнь тени $C2!",ch,0,victim,TO_NOTVICT,POS_RESTING);
  act_p("$c1 дает жизнь твоей тени!", ch, 0, victim, TO_VICT,POS_RESTING);

  victim->getPC()->shadow        = ch->getModifyLevel() / 10;

    postaffect_to_char( ch, sn, 24 );
}


AFFECT_DECL(EvilSpirit);
VOID_AFFECT(EvilSpirit)::update( Character *ch, Affect *paf ) 
{
    DefaultAffectHandler::update( ch, paf );

    if (ch->getClan( ) == clan_invader)
        return;
    
    Spell::Pointer spell = gsn_mental_knife->getSpell( );

    if (spell)
        spell->run( ch, ch, gsn_mental_attack, ch->getModifyLevel( ) );
}

VOID_AFFECT(EvilSpirit)::update( Room *room, Affect *paf ) 
{
    Affect af;
    Character *vch;

    af.where        = TO_AFFECTS;
    af.type         = gsn_evil_spirit;
    af.level         = paf->level;
    af.duration        = number_range(1,(af.level/30));
    af.location        = APPLY_NONE;
    af.modifier        = 0;
    af.bitvector= 0;

    for ( vch = room->people; vch; vch = vch->next_in_room )
    {
        if ( !saves_spell(vch->getModifyLevel() + 2, vch, DAM_MENTAL, 0, DAMF_SPELL)
                && !vch->is_immortal()
                && !is_safe_rspell(vch->getModifyLevel() + 2, vch)
                && !vch->isAffected(gsn_evil_spirit) && number_bits(3) == 0 )
        {
            vch->send_to("Злые духи овладевают тобой.\n\r");
            act_p("Злые духи овладевают $c1.",vch,0,0,TO_ROOM,POS_RESTING);
            affect_join(vch,&af);
        }
    }
}

VOID_AFFECT(EvilSpirit)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Злые духи воцарились здесь на {W%1$d{x ча%1$Iс|са|сов.", paf->duration )
        << endl;
}



/*-----------------------------------------------------------------
 * 'darkleague' command 
 *----------------------------------------------------------------*/
COMMAND(CDarkLeague, "darkleague")
{
    PCharacter *pch;
    const ClanOrgs *orgs;
    DLString arguments, cmd, arg;

    if (ch->is_npc( ))
        return;

    pch = ch->getPC( );
    
    if (pch->getClan( ) != clan_invader) {
        pch->println( "Ты не принадлежишь к клану Захватчиков." );
        return;
    }
    
    if (!( orgs = clan_invader->getOrgs( ) )) {
        pch->println( "Попробуй позже." );
        return;
    }

    if (!pch->getClan( )->isRecruiter( pch )) {
        pch->println( "Твоих полномочий недостаточно." );
        return;
    }

    arguments = constArguments;
    cmd = arguments.getOneArgument( );
    arg = arguments.getOneArgument( );
    
    if (cmd.empty( )) {
        doUsage( pch );
    }
    else if (arg_is_list( cmd )) {
        orgs->doList( pch );
    }
    else if (arg_oneof( cmd, "induct", "принять" )) {
        if (arg_is_self( arg ))
            orgs->doSelfInduct( pch, arguments );
        else
            orgs->doInduct( pch, arg );
    }
    else if (arg_oneof( cmd, "remove", "выгнать", "уйти" )) {
        if (arg_is_self( arg ))
            orgs->doSelfRemove( pch );
        else
            orgs->doRemove( pch, arg );
    }
    else if (arg_oneof( cmd, "members", "члены" )) {
        orgs->doMembers( pch );
    }
    else {
        doUsage( pch );
    }
}

void CDarkLeague::doUsage( PCharacter *pch )
{
    ostringstream buf;

    buf << "Для руководства: " << endl
        << "{wdarkleague list{x            - посмотреть список групп" << endl
        << "{wdarkleague members{x         - посмотреть список членов группы" << endl
        << "{wdarkleague remove self{x     - выйти из группы" << endl
        << "{wdarkleague induct <{Dname{w>{x - принять кого-то в группу" << endl
        << "{wdarkleague remove <{Dname{w>{x - выгнать кого-то из группы" << endl
        << endl
        << "Для лидера: " << endl
        << "{wdarkleague induct self <{Dname{w>{x - принять себя в группу" << endl;

    pch->send_to( buf );
}


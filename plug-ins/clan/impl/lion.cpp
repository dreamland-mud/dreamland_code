/* $Id: lion.cpp,v 1.1.6.9.6.17 2010-09-01 21:20:44 rufina Exp $
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

#include "lion.h"

#include "summoncreaturespell.h"
#include "affecthandlertemplate.h"
#include "spelltemplate.h"                                                 
#include "skillcommandtemplate.h"
#include "skill.h"
#include "skillmanager.h"

#include "race.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"
#include "affect.h"

#include "act.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "move_utils.h"
#include "vnum.h"
#include "clanreference.h"
#include "mercdb.h"
#include "handler.h"
#include "fight.h"
#include "magic.h"
#include "def.h"

CLAN(hunter);
GSN(dispel_affects);

#define OBJ_VNUM_EYED_SWORD                503        
#define OBJ_VNUM_LION_SHIELD                33

/*--------------------------------------------------------------------------
 * LionMan 
 *-------------------------------------------------------------------------*/
void ClanGuardLion::actGreet( PCharacter *wch )
{
    do_say(ch, "Добро пожаловать, странник.");
}
void ClanGuardLion::actPush( PCharacter *wch )
{
    act( "$C1 выпускает когти.\n\rИ ты быстренько убираешься из этой местности.", wch, 0, ch, TO_CHAR );
    act( "$C1 глядя на $c4 выпускает когти и $c1 сматывает удочки.", wch, 0, ch, TO_ROOM );
}
int ClanGuardLion::getCast( Character *victim )
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
                sn = gsn_acid_blast;
                break;
        case  4: 
        case  5:
                sn = gsn_caustic_font;
                break; 
        case  6:
        case  7:
        case  8:
                sn = gsn_acid_arrow;
                break;
        default:
                sn = -1;
                break;
        }

        return sn;
}

bool ClanGuardLion::specFight( )
{
        Character *victim;

        if (!( victim = getVictim( ) ))
            return true;

        if ( number_percent() < 33 )
        {
                int damage_claw;

                damage_claw = dice(ch->getRealLevel(), 24) + ch->damroll;
                damage(ch, victim, damage_claw, gsn_claw, DAM_BASH, true, DAMF_WEAPON);
                return true;
        }

        spec_cast( victim );
        return true;
}


/*
 * 'claw' skill command
 */

SKILL_RUNP( claw )
{
        Character *victim;
        int chance;
        int damage_claw;

        if (!gsn_claw->available( ch ) )
        {
            ch->send_to("Ась?\n\r");
            return;
        }

        if (!gsn_claw->usable( ch ))
                return;

        if ( ( victim = ch->fighting ) == 0 )
        {
                ch->send_to("Сейчас ты не сражаешься.\n\r");
                return;
        }
         
        if ( victim == ch )
        {
                ch->send_to("Ты же не хочешь и в самом деле отхватить себе голову?\n\r");
                return;
        }

        if (is_safe(ch,victim))
                return;

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act_p("Но ведь $C1 твой друг!",ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }
   
        // size and weight
        chance = gsn_claw->getEffective( ch );
        chance += min(ch->canCarryWeight( ), ch->carry_weight) / 100;
        chance += ch->size;

        // stats
        chance += ch->getCurrStat(STAT_STR);
        chance -= victim->getCurrStat(STAT_DEX) * 2;

        if (is_flying( ch ))
                chance -= 10;

        // speed
        if (IS_QUICK(ch))
                chance += 10;

        if (IS_QUICK(victim))
                chance -= 20;
                
        if ( ch->hit <= ch->max_hit / 6 )
                chance += 30;
        

        // level
        // chance += (ch->level - victim->level) * 2;

        if ( ch->mana < gsn_claw->getMana( ) )
        {
                ch->send_to("Твоих сил недостаточно для этого.\n\r");
                return;
        }
    
        // now the attack
        if ( number_percent() < chance )
        {
                ch->mana -= gsn_claw->getMana( );

                gsn_claw->improve( ch, true, victim );
                ch->setWait(gsn_claw->getBeats( ));
                victim->position = POS_RESTING;
                damage_claw = dice(ch->getModifyLevel(), 24) + ch->damroll;
                if ( ch->hit <= ch->max_hit / 6 )
                        damage_claw += ch->damroll;
                damage(ch,victim,damage_claw,gsn_claw, DAM_BASH, true, DAMF_WEAPON);
        }
        else
        {
                ch->mana -= 50;
                damage(ch,victim,0,gsn_claw,DAM_BASH, true, DAMF_WEAPON);
                gsn_claw->improve( ch, false, victim );
                ch->position = POS_RESTING;
                ch->setWait(gsn_claw->getBeats( ));
        }
}



SPELL_DECL(EvolveLion);
VOID_SPELL(EvolveLion)::run( Character *ch, Character *, int sn, int level ) 
{ 
  Affect af;

  if (ch->is_npc())
      return;

  if ( ch->isAffected(sn )
                || ch->hit > ch->max_hit )
        {
                ch->send_to("Ты не можешь быть более Львом, чем сейчас.\n\r");
                return;
        }

  ch->hit += ch->getPC()->perm_hit / 2;

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level; 
  af.duration  = 3 + level / 30;
  af.location  = APPLY_HIT;
  af.modifier  = ch->getPC()->perm_hit / 2;
  af.bitvector = 0;
  affect_to_char(ch,&af);

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 3 + level / 30;
  af.location  = APPLY_DEX;
  af.modifier  = -(1 + level / 20);
  af.bitvector = AFF_SLOW;
  affect_to_char( ch, &af );

  af.where     = TO_AFFECTS;
  af.type      = sn;
  af.level     = level;
  af.duration  = 3 + level / 30;
  af.location  = APPLY_DAMROLL;
  af.modifier  = level / 2;
  af.bitvector = AFF_BERSERK;
  affect_to_char( ch, &af );

  act_p("Ты чувствуешь себя немного неповоротлив$gым|ым|ой, но зато намного более сильн$gым|ым|ой.",
                ch,0,0,TO_CHAR,POS_RESTING);
  act_p( "Кожа $c2 становится серой!",ch,0,0,TO_ROOM,POS_RESTING);

}

/*
 * eyed sword behavior
 */
void LionEyedSword::wear( Character *ch )
{
    act_p("Глаза $o2 открываются.",ch,obj,0,TO_CHAR,POS_RESTING);
    act_p("Глаза $o2 открываются.",ch,obj,0,TO_ROOM,POS_RESTING);

}
void LionEyedSword::equip( Character *ch )
{
    short level = ch->getModifyLevel();

    if (  level <= 10)                           obj->value[2] = 3;
    else if ( level > 10 && level <= 20)   obj->value[2] = 4;
    else if ( level > 20 && level <= 30)   obj->value[2] = 5;
    else if ( level > 30 && level <= 40)   obj->value[2] = 6;
    else if ( level > 40 && level <= 50)   obj->value[2] = 7;
    else if ( level > 50 && level <= 60)   obj->value[2] = 8;
    else if ( level > 60 && level <= 70)   obj->value[2] = 9;
    else if ( level > 70 && level <= 80)   obj->value[2] = 10;
    else                                   obj->value[2] = 11;

    obj->level = ch->getRealLevel( );
}



SPELL_DECL(EyedSword);
VOID_SPELL(EyedSword)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_STRING_LENGTH];
    Object *eyed;

    eyed        = create_object(get_obj_index(OBJ_VNUM_EYED_SWORD), 0);
    eyed->setOwner( ch->getNameP( ) );
    eyed->from = str_dup(ch->getNameP( ));
    eyed->level = ch->getRealLevel( );
    eyed->fmtShortDescr( eyed->getShortDescr( ), ch->getNameP( ) );
    eyed->fmtDescription( eyed->getDescription( ), ch->getNameP( ) );

    sprintf( buf, eyed->pIndexData->extra_descr->description, ch->getNameP( ) );
    eyed->addExtraDescr( eyed->pIndexData->extra_descr->keyword, buf );

    eyed->value[2] = ( ch->getModifyLevel() / 10) + 3;
    eyed->level = ch->getRealLevel( );
    eyed->cost = 0;
    obj_to_char( eyed, ch);
    ch->send_to("Ты создаешь меч с ТВОИМ именем.\n\r");
    ch->send_to("Не забудь, что ты больше не сможешь создать это оружие.\n\r");
}

SPELL_DECL(EyesOfTiger);
VOID_SPELL(EyesOfTiger)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if (DIGGED(victim))
        {
                ch->send_to("Твой львиный глаз не может найти такого.\n\r");
                return;
        }

        if (victim->is_npc() || victim->getPC()->getClan() != clan_hunter)
        {
                ch->send_to("Ты можешь следить только за Охотниками!\n\r");
                return;
        }
        
        if (is_safe_nomessage(ch,victim)) 
        {
                ch->send_to("Твой львиный глаз не смог найти такого.\n\r");
                return;
        }
        
        do_look_auto( ch, victim->in_room );
}


SPELL_DECL(LionShield);
VOID_SPELL(LionShield)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
  Object *shield;
  Affect af;

  shield = create_object( get_obj_index(OBJ_VNUM_LION_SHIELD), level );
  shield->timer = level;
  shield->cost  = 0;
  obj_to_char(shield, ch);
  
  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = level / 8;
  af.bitvector    = 0;

  af.location     = APPLY_HITROLL;
  affect_to_obj( shield, &af);

  af.location     = APPLY_DAMROLL;
  affect_to_obj( shield, &af);

  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = -level/2;
  af.bitvector    = 0;
  af.location     = APPLY_AC;
  affect_to_obj( shield, &af);

  af.where        = TO_OBJECT;
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;
  af.modifier     = max(1,level /  30);
  af.bitvector    = 0;
  af.location     = APPLY_CHA;
  affect_to_obj( shield, &af);

  act_p( "Ты создаешь $o4!",ch,shield,0,TO_CHAR,POS_RESTING );
  act_p( "$c1 создает $o4!",ch,shield,0,TO_ROOM,POS_RESTING );

}



SPELL_DECL_T(Panthers, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, Panthers)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
                         ch->hit, 
                         (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                         number_range(level/15, level/10),
                         number_range(level/4, level/2),
                         number_range(level/9, level/6) );
} 

SPELL_DECL(Prevent);
VOID_SPELL(Prevent)::run( Character *ch, Character *victim, int sn, int level ) 
{
    Affect af;

    if (ch->isAffected( sn )) {
        act("Ты уже защище$gно|н|на от ловушек Охотников.", ch, 0, 0, TO_CHAR);
        return;
    }

    af.where                = TO_AFFECTS;
    af.type               = sn;
    af.level              = level; 
    af.duration           = max( 6, ch->getPC( )->getClanLevel( ) * 2 );
    af.bitvector          = 0;
    af.modifier           = 0;
    af.location           = APPLY_NONE;
    affect_to_char(ch, &af);  

    ch->println( "Ты защищаешь себя от ловушек Охотников." );
}

VOID_SPELL(Prevent)::run( Character *ch, Room *room, int sn, int level ) 
{ 
        Affect af;

        if ( room->isAffected( sn ))
        {
                ch->send_to("Это место уже защищено от мести и ловушек Охотников.\n\r");
                return;
        }

        af.where     = TO_ROOM_AFFECTS;
        af.type      = sn;
        af.level     = level;
        af.duration  = max( 1, ch->getPC( )->getClanLevel( ) * 1 );
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = AFF_ROOM_PREVENT;
        room->affectTo( &af );

        ch->send_to( "Ты защищаешь местность от ловушек Охотников и от их мести.\n\r");
        act_p( "$c1 защищает местность от ловушек Охотников и от их мести.",ch,0,0,TO_ROOM,POS_RESTING);
}

AFFECT_DECL(Prevent);
VOID_AFFECT(Prevent)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Местность на {W%1$d{x ча%1$Iс|са|сов защищена от ловушек и мести Охотников.", paf->duration )
        << endl;
}



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
#include "skill_utils.h"

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
    act("%2$^C1 выпускает когти.\n\rИ ты быстренько убираешься из этой местности.", wch, ch, 0,TO_CHAR);
    oldact("$C1, глядя на $c4, выпускает когти, и $c1 сматывает удочки.", wch, 0, ch, TO_ROOM );
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
            ch->pecho("Это умение тебе недоступно.");
            return;
        }

        if (!gsn_claw->usable( ch ))
                return;

        if ( ( victim = ch->fighting ) == 0 )
        {
                ch->pecho("Сейчас ты не сражаешься.");
                return;
        }
         
        if ( victim == ch )
        {
                ch->pecho("Ты же не хочешь и в самом деле отхватить себе голову?");
                return;
        }

        if (is_safe(ch,victim))
                return;

        if (IS_CHARMED(ch) && ch->master == victim)
        {
                act("Но ведь %2$C1 твой друг!",ch,victim,0,TO_CHAR);
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
        chance += skill_level_bonus(*gsn_claw, ch);
        // chance += (ch->level - victim->level) * 2;

        if ( ch->mana < gsn_claw->getMana( ) )
        {
                ch->pecho("У тебя не хватает энергии.");
                return;
        }
    
        // now the attack
        if ( number_percent() < chance )
        {
                ch->mana -= gsn_claw->getMana( );

                gsn_claw->improve( ch, true, victim );
                ch->setWait(gsn_claw->getBeats( ));
                victim->setWaitViolence( 2 );            
                victim->position = POS_RESTING;
                damage_claw = dice(skill_level(*gsn_claw, ch) , 24) + ch->damroll;
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

  if ( ch->isAffected(sn ) )
        {
                ch->pecho("Ты уже трансформирова{Smлся{Sfлась{Sx во льва.");
                return;
        }

  ch->hit += ch->getPC()->perm_hit / 2;

  af.type      = sn;
  af.level     = level; 
  af.duration  = 3 + level / 30;
  af.location = APPLY_HIT;
  af.modifier  = ch->getPC()->perm_hit / 2;
  affect_to_char(ch,&af);

  af.bitvector.setTable(&affect_flags);
  af.type      = sn;
  af.level     = level;
  af.duration  = 3 + level / 30;
  af.location = APPLY_DEX;
  af.modifier  = -(1 + level / 20);
  af.bitvector.setValue(AFF_SLOW);
  affect_to_char( ch, &af );

  af.bitvector.setTable(&affect_flags);
  af.type      = sn;
  af.level     = level;
  af.duration  = 3 + level / 30;
  af.location = APPLY_DAMROLL;
  af.modifier  = level / 2;
  af.bitvector.setValue(AFF_BERSERK);
  affect_to_char( ch, &af );

  oldact("Ты чувствуешь себя немного неповоротлив$gым|ым|ой, но зато намного более сильн$gым|ым|ой.", ch,0,0,TO_CHAR);
  act("Кожа %C2 становится серой!",ch,0,0,TO_ROOM);

}

/*
 * eyed sword behavior
 */
void LionEyedSword::wear( Character *ch )
{
    act("Глаза %3$O2 открываются.",ch,0,obj,TO_CHAR);
    act("Глаза %3$O2 открываются.",ch,0,obj,TO_ROOM);

}
void LionEyedSword::equip( Character *ch )
{
    short level = ch->getModifyLevel();

    if (  level <= 10)                           obj->value2(3);
    else if ( level > 10 && level <= 20)   obj->value2(4);
    else if ( level > 20 && level <= 30)   obj->value2(5);
    else if ( level > 30 && level <= 40)   obj->value2(6);
    else if ( level > 40 && level <= 50)   obj->value2(7);
    else if ( level > 50 && level <= 60)   obj->value2(8);
    else if ( level > 60 && level <= 70)   obj->value2(9);
    else if ( level > 70 && level <= 80)   obj->value2(10);
    else                                   obj->value2(11);

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

    eyed->value2(( ch->getModifyLevel() / 10) + 3);
    eyed->level = ch->getRealLevel( );
    eyed->cost = 0;
    obj_to_char( eyed, ch);
    ch->pecho("Ты создаешь меч с ТВОИМ именем.");
    ch->pecho("Не забудь, что ты больше не сможешь создать это оружие.");
}

SPELL_DECL(EyesOfTiger);
VOID_SPELL(EyesOfTiger)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if (DIGGED(victim))
        {
                ch->pecho("Твой львиный глаз не может найти такого.");
                return;
        }

        if (victim->is_npc() || victim->getPC()->getClan() != clan_hunter)
        {
                ch->pecho("Ты можешь следить только за Охотниками!");
                return;
        }
        
        if (is_safe_nomessage(ch,victim)) 
        {
                ch->pecho("Твой львиный глаз не смог найти такого.");
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
  
  af.type         = sn;
  af.level        = level;
  af.duration     = -1;

  af.modifier     = level / 8;
  af.location = APPLY_HITROLL;
  affect_to_obj( shield, &af);

  af.location = APPLY_DAMROLL;
  affect_to_obj( shield, &af);

  af.modifier     = -level/2;
  af.location = APPLY_AC;
  affect_to_obj( shield, &af);

  af.modifier     = max(1,level /  30);
  af.location = APPLY_CHA;
  affect_to_obj( shield, &af);

  act("Ты создаешь %3$O4!",ch,0,shield,TO_CHAR);
  act("%1$^C1 создает %3$C4!",ch,0,shield,TO_ROOM);

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
        oldact("Ты уже защище$gно|н|на от ловушек Охотников.", ch, 0, 0, TO_CHAR);
        return;
    }

    af.type               = sn;
    af.level              = level; 
    af.duration           = max( 6, ch->getPC( )->getClanLevel( ) * 2 );
    
    affect_to_char(ch, &af);  

    ch->pecho( "Ты защищаешь себя от ловушек Охотников." );
}

VOID_SPELL(Prevent)::run( Character *ch, Room *room, int sn, int level ) 
{ 
        Affect af;

        if ( room->isAffected( sn ))
        {
                ch->pecho("Это место уже защищено от мести и ловушек Охотников.");
                return;
        }

        af.bitvector.setTable(&raffect_flags);
        af.type      = sn;
        af.level     = level;
        af.duration  = max( 1, ch->getPC( )->getClanLevel( ) * 1 );
        
        af.modifier  = 0;
        af.bitvector.setValue(AFF_ROOM_PREVENT);
        room->affectTo( &af );

        ch->pecho("Ты защищаешь местность от ловушек Охотников и от их мести.");
        act("%^C1 защищает местность от ловушек Охотников и от их мести.",ch,0,0,TO_ROOM);
}

AFFECT_DECL(Prevent);
VOID_AFFECT(Prevent)::toStream( ostringstream &buf, Affect *paf ) 
{
    buf << fmt( 0, "Местность на {W%1$d{x ча%1$Iс|са|сов защищена от ловушек и мести Охотников.", paf->duration )
        << endl;
}



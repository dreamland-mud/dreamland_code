/* $Id: battlerager.cpp,v 1.1.6.10.4.18 2010-09-01 21:20:44 rufina Exp $
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

#include "battlerager.h"
#include "xmlattributerestring.h"
#include "logstream.h"

#include "commandtemplate.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"

#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

using std::max;
using std::min;

Object * bodypart_create( int vnum, Character *ch, Object *corpse );

#define OBJ_VNUM_BATTLE_PONCHO       26

CLAN(battlerager);

/*
 * poncho behavior
 */
void BattleragerPoncho::wear( Character *ch ) 
{
    Affect af;
    short level = ch->getModifyLevel( );

    if (ch->isAffected(gsn_haste ) || ch->isAffected(gsn_transform )) 
        return;

    af.where = TO_AFFECTS;
    af.type = gsn_haste;
    af.duration = -2;
    af.level = level;
    af.bitvector = AFF_HASTE;
    af.location = APPLY_DEX;
    af.modifier = 1 + ( level >= 18 ) + ( level >= 30 ) + ( level >= 45 );
    affect_to_char(ch, &af);
}

void BattleragerPoncho::remove( Character *ch )
{
    if (ch->isAffected(gsn_haste))
        affect_strip(ch, gsn_haste);
}

PersonalBattleragerPoncho::~PersonalBattleragerPoncho( )
{
}


/*
 * 'chop' skill command
 * 'chop leg|arm|head|рука|нога|голова <corpse>'
 */
COMMAND(CChop, "chop")
{
    Object *corpse, *axe;
    bitstring_t part;
    DLString args = constArguments, argPart, argBody;
    int vnum;

    if (!gsn_trophy->available( ch )) {
        ch->println( "Ты не умеешь рассекать трупы." );
        return;
    }

    if (!gsn_trophy->usable( ch ))
        return;

    argPart = args.getOneArgument( );
    argBody = args.getOneArgument( );

    if (argPart.empty( )) {
        ch->println( "Какую часть тела ты хочешь отрубить - ногу, голову или руку?" );
        return;
    }

    if (arg_oneof( argPart, "leg", "нога", "ногу" )) {
        part = PART_LEGS;
        vnum = OBJ_VNUM_SLICED_LEG;
    }
    else if (arg_oneof( argPart, "arm", "рука", "руку" )) {
        part = PART_ARMS;
        vnum = OBJ_VNUM_SLICED_ARM;
    }
    else if (arg_oneof( argPart, "head", "голова", "голову" )) {
        part = PART_HEAD;
        vnum = OBJ_VNUM_SEVERED_HEAD;
    }
    else {
        ch->println( "Ты можешь отрубить только ногу, голову или руку." );
        return;
    }

    if (argBody.empty( )) {
        ch->println( "От какого трупа ты хочешь отрубить кусок?" );
        return;
    }

    if (!( corpse = get_obj_here( ch, argBody.c_str( ) ) )) {
        ch->println( "Здесь нет такого трупа." );
        return;
    }

    if (corpse->item_type != ITEM_CORPSE_PC && corpse->item_type != ITEM_CORPSE_NPC) {
        ch->println( "Это не труп." );
        return;
    }

    if (!IS_SET(corpse->value2(), part)) {
        ch->pecho( "У этого трупа нету %s.", 
                   part_flags.messages( part, true, '2' ).c_str( ) );
        return;
    }
    
    if (!( axe = get_wield( ch, false ) )) {
        ch->println( "Чем рубить будешь?" );
        return;
    }

    if (axe->value3() != DAMW_SLASH && axe->value3() != DAMW_CHOP && axe->value3() != DAMW_SLICE) {
        ch->println( "Твоим оружием неудобно это делать." );
        return;
    }

    corpse->value2(corpse->value2() & ~part);
    ch->setWait( gsn_trophy->getBeats( ) / 2 );

    DLString what = part_flags.messages( part, true, '4' );

    if (number_percent( ) > 2 * gsn_trophy->getEffective( ch ) / 3) {
        ch->pecho( "Ты лупишь оружием плашмя, превращая %s трупа в кровавое месиво.",
                   what.c_str( ) );
        ch->recho( "%^C1 плашмя лупит оружием по %O3.", ch, corpse ); 
        gsn_trophy->improve( ch, false );
        return;
    }

    ch->pecho( "Ты отсекаешь %s от %O2.", what.c_str( ), corpse );
    ch->recho( "%^C1 отсекает %s от %O2.", ch, what.c_str( ), corpse );
    bodypart_create( vnum, 0, corpse );
    gsn_trophy->improve( ch, true );
}

bool CChop::visible( Character *ch ) const
{
    return !ch->is_npc() && ch->getPC()->getClan() == clan_battlerager;
}

/*
 * 'trophy' skill command
 */
SKILL_RUNP( trophy )
{
    int trophy_vnum;
    Object *trophy;
    Affect af;
    Object *part;
    char arg[MAX_INPUT_LENGTH];
    short level;
    int mana = gsn_trophy->getMana( );

    argument = one_argument( argument, arg );

    if (!gsn_trophy->available( ch )) {
        ch->println( "Ась?" );
        return;
    }

    if (!gsn_trophy->usable( ch ))
        return;

    if (ch->isAffected(gsn_trophy))
    {
        ch->println( "Но у тебя уже есть один трофей!" );
        return;
    }

    if (ch->mana < mana)
    {
        ch->println( "Ты слишком слаб, чтоб сконцентрироваться." );
        return;
    }

    if (arg[0] == '\0')
    {
        ch->println( "Что именно ты хочешь превратить в трофей?" );
        return;
    }

    if ( ( part = get_obj_carry( ch, arg ) ) == 0 )
    {
        ch->println( "У тебя нету такой части тела." );
        return;
    }


    switch (part->pIndexData->vnum) {
    case OBJ_VNUM_SLICED_ARM:
    case OBJ_VNUM_SLICED_LEG:
    case OBJ_VNUM_SEVERED_HEAD:
    case OBJ_VNUM_TORN_HEART:
    case OBJ_VNUM_GUTS:
        trophy_vnum = OBJ_VNUM_BATTLE_PONCHO;
    break;
    case OBJ_VNUM_BRAINS:
        ch->println( "А почему бы тебе просто не съесть их?" );
        return;
    default:
        ch->println( "Ты не можешь превратить это в трофей!" );
        return;
    }

    if (part->from[0] == '\0')
    {
        ch->println( "Это какая-то неправильная часть тела." );
        return;
    }

    if (part->level < ch->getModifyLevel( ) - 20) {
        ch->println( "Эта часть тела слишком мала для трофея." );
        return;
    }

    if (number_percent( ) > (gsn_trophy->getEffective( ch )/3)*2)
    {
        ch->println( "Твоя попытка не удалась, и ты разрушаешь это." );
        extract_obj(part);
        return;
    }

    ch->setWait( gsn_trophy->getBeats( ) );

    if (!ch->is_npc() && number_percent() < gsn_trophy->getEffective( ch ))
    {
        af.where  = TO_AFFECTS;
        af.type        = gsn_trophy;
        af.level        = ch->getModifyLevel();
        af.duration        = ch->getModifyLevel() / 2;
        af.modifier        = 0;
        af.bitvector         = 0;

        af.location        = 0;
        affect_to_char(ch,&af);

        if ( trophy_vnum != 0 )
        {
            level = min(part->level + 5, MAX_LEVEL);

            trophy = create_object( get_obj_index( trophy_vnum ), level );
            trophy->timer = ch->getModifyLevel() * 2;
            trophy->fmtShortDescr( trophy->getShortDescr( ), part->from );
            trophy->fmtDescription( trophy->getDescription( ), part->from );
            dress_created_item( gsn_trophy, trophy, ch, argument );

            trophy->cost  = 0;
            trophy->level = ch->getRealLevel( );
            ch->mana     -= mana;
            af.where        = TO_OBJECT;
            af.type         = gsn_trophy;
            af.level        = level;
            af.duration        = -1;
            af.location        = APPLY_DAMROLL;
            af.modifier   = ch->applyCurse( ch->getModifyLevel( ) / 5 );
            af.bitvector        = 0;
            affect_to_obj( trophy, &af );

            af.location        = APPLY_HITROLL;
            af.modifier   = ch->applyCurse( ch->getModifyLevel( ) / 5 );
            af.bitvector        = 0;
            affect_to_obj( trophy, &af );

            af.location        = APPLY_INT;
            af.modifier        = level > 20 ? -2 : -1;
            affect_to_obj( trophy, &af );

            af.location        = APPLY_STR;
            af.modifier        = level > 20 ? 2 : 1;
            affect_to_obj( trophy, &af );

            trophy->value0(ch->getModifyLevel());
            trophy->value1(ch->getModifyLevel());
            trophy->value2(ch->getModifyLevel());
            trophy->value3(ch->getModifyLevel());


            obj_to_char(trophy, ch);
            gsn_trophy->improve( ch, true );

            act_p("Ты изготавливаешь пончо из $o2!",ch,part,0,TO_CHAR,POS_RESTING);
            act_p("$c1 изготавливает пончо из $o2!",ch,part,0,TO_ROOM,POS_RESTING);

            extract_obj(part);
            return;
        }
    }
    else
    {
        ch->println( "Ты разрушаешь это." );
        extract_obj(part);
        ch->mana -= mana / 2;
        gsn_trophy->improve( ch, false );
    }
}

/*
 * 'mortal strike' skill command
 */
SKILL_DECL( mortalstrike );
BOOL_SKILL( mortalstrike )::run( Character *ch, Character *victim )
{
    Object *wield;
    int chance, learned;
    int weaponLevelDiff;

    if (!gsn_mortal_strike->usable( ch, false ))
        return false;

    if ((learned = gsn_mortal_strike->getEffective( ch )) <= 1)
        return false;

    // Works only for primary weapon.
    if ((wield = get_eq_char(ch, wear_wield)) == 0)
        return false;

    // Low-level weapon cannot strike a powerful victim.
    // However, allow weapon level as low as 80+ for heroes.
    weaponLevelDiff = max(1, ch->getModifyLevel() / 5); 
    if (victim->getModifyLevel() - wield->level > weaponLevelDiff) {
        notice("[mortal strike] %s (%dlvl) vs %s (%dlvl), weapon level %d, diff %d.",
                ch->getNameP('1').c_str(), ch->getModifyLevel(),
                victim->getNameP('1').c_str(), victim->getModifyLevel(),
                wield->level, weaponLevelDiff);
        return false;
    }

    // Calculate real chance to strike (original Anatolia code, plus clan level bonus).
    // For hero PK, chances are ranging from 1+clanLevel to 8+clanLevel, i.e. from 1 to 16.
    // For players of the same level, chances are ranging from 2 to 12.
    chance = 1 + learned / 30; 
    chance += (ch->getModifyLevel() - victim->getModifyLevel()) / 2;
    chance += ch->is_npc() ? 0 : ch->getPC()->getClanLevel();
    chance = max(1, chance);
    notice("[mortal strike] %s (%dlvl) vs %s (%dlvl), learned %d, chance %d, level diff %d.",
            ch->getNameP('1').c_str(), ch->getModifyLevel(),
            victim->getNameP('1').c_str(), victim->getModifyLevel(),
            learned, chance, (ch->getModifyLevel() - victim->getModifyLevel()) / 2);

    // Dice roll failed, learn from mistakes.
    if (number_percent() > chance) {
        gsn_mortal_strike->improve( ch, false, victim );
        return false;
    }

    // Success, inflict a lot of damage. Anatolia implementation had (victim->hit+1), but the 
    // resulting damage always got reduced by sanctuary and other protections.
    int dam;
    act_p("{RТвой молниеносный удар в одно мгновение лишает $C4 жизни!{x", ch,0,victim,TO_CHAR,POS_RESTING);
    act_p("{RМолниеносный удар $c2 в одно мгновение лишает $C4 жизни!{x", ch,0,victim,TO_NOTVICT,POS_RESTING);
    act_p("{RМолниеносный удар $c2 в одно мгновение лишает тебя жизни!{x", ch,0,victim,TO_VICT,POS_DEAD);
    dam = victim->hit * 2; 
    dam *= ch->getPC( )->curse / 100;
    notice("[mortal strike] %s (%dlvl) vs %s (%dlvl), damage %d, victim hp %d/%d.",
            ch->getNameP('1').c_str(), ch->getModifyLevel(),
            victim->getNameP('1').c_str(), victim->getModifyLevel(),
            dam, victim->hit, victim->max_hit);
    damage(ch, victim, dam, gsn_mortal_strike, attack_table[wield->value3()].damage, true);
    gsn_mortal_strike->improve( ch, true, victim );
    return true;
}

/*
 * 'bloodthirst' skill command
 */

SKILL_RUNP( bloodthirst )
{
    int chance, hp_percent;

    if (!gsn_bloodthirst->available( ch ))
    {
        ch->println( "Ты не знаешь что такое жажда." );
        return;
    }

    if (!gsn_bloodthirst->usable( ch ))
      return;

    chance = gsn_bloodthirst->getEffective( ch );

    if (IS_AFFECTED(ch,AFF_BLOODTHIRST) || ch->isAffected(gsn_bloodthirst) )
    {
        ch->println( "Ты уже давно жаждешь крови." );
        return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
        ch->println( "Ты слишком миролюбив, чтоб жаждать крови." );
        return;
    }

    if (ch->fighting == 0)
      {
        ch->println( "Для этого ты должен сражаться." );
        return;
      }

    /* modifiers */

    hp_percent = ch->applyCurse( HEALTH(ch) );
    chance += ch->applyCurse( 25 - hp_percent / 2 );

    if (number_percent() < chance)
    {
        Affect af;

        ch->setWaitViolence( 1 );


        ch->println( "Ты жаждешь {rкрови!{x" );
        act_p("Глаза $c2 загораются кровожадным огнем.",
               ch,0,0,TO_ROOM,POS_RESTING);
        gsn_bloodthirst->improve( ch, true );

        af.where        = TO_AFFECTS;
        af.type                = gsn_bloodthirst;
        af.level        = ch->getModifyLevel();
        af.duration        = 2 + ch->getModifyLevel() / 18;
        af.modifier        = ch->applyCurse( 5 + ch->getModifyLevel( ) / 4 );
        af.bitvector         = AFF_BLOODTHIRST;

        af.location        = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location        = APPLY_DAMROLL;
        affect_to_char(ch,&af);

        af.modifier        = ch->applyCurse( -min( ch->getModifyLevel( ) - 5, 35 ) );
        af.location        = APPLY_AC;
        affect_to_char(ch,&af);
    }

    else
    {
        ch->setWaitViolence( 3 );

        ch->println( "На миг ты чувствуешь себя кровожадно, но это быстро проходит." );
        gsn_bloodthirst->improve( ch, false );
    }
}


/*
 * 'spellbane' skill command
 */

SKILL_RUNP( spellbane )
{
        Affect af;
        
        if (!gsn_spellbane->usable( ch ))
            return;

        if (ch->isAffected(gsn_spellbane))
        {
                ch->println( "Ты уже отражаешь заклинания." );
                return;
        }

        ch->setWait( gsn_spellbane->getBeats( )  );

        af.where        = TO_AFFECTS;
        af.type                = gsn_spellbane;
        af.level        = ch->getModifyLevel();
        af.duration        = ch->getModifyLevel() / 3;
        af.location        = APPLY_SAVING_SPELL;
        af.modifier        = ch->applyCurse( -ch->getModifyLevel( ) / 4 );
        af.bitvector        = 0;

        affect_to_char(ch,&af);

        act_p("Ненависть к магии окружает тебя.",ch,0,0,TO_CHAR,POS_RESTING);
        act_p("$c1 распространяет вокруг себя ненависть к магии.", ch,0,0,TO_ROOM,POS_RESTING);
}

/*
 * 'resistance' skill command
 */

SKILL_RUNP( resistance )
{
        int mana = gsn_resistance->getMana( );

        //if (!gsn_resistance->available( ch ))
        //        return;

        if (!gsn_resistance->usable( ch ))
                return;

        if (ch->isAffected(gsn_resistance))
        {
                ch->println( "Ты уже защищен. Больше уже некуда." );
                return;
        }

        if ( ch->mana < mana )
        {
                ch->println( "У тебя недостаточно энергии для этого." );
                return;
        }

        ch->setWait( gsn_resistance->getBeats( )  );

        if ((!ch->is_npc() && number_percent() < gsn_resistance->getEffective( ch ))
          || ch->is_npc() )
    {
      Affect af;

      af.where        = TO_AFFECTS;
      af.type         = gsn_resistance;
      af.level         = ch->getModifyLevel();
      af.duration = ch->getModifyLevel() / 6;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;

      affect_to_char(ch,&af);
      ch->mana -= mana;

      act_p("Ты чувствуешь себя крепче!",ch,0,0,TO_CHAR,POS_RESTING);
      act_p("$c1 выглядит покрепче.",ch,0,0,TO_ROOM,POS_RESTING);
      gsn_resistance->improve( ch, true );
    }
  else
    {
      ch->mana -= mana / 2;

     ch->println( "Ты напрягаешь свои мускулы, но это все впустую." );
      act_p("$c1 играет мускулами, пытаясь выглядеть крепче.",
             ch,0,0,TO_ROOM,POS_RESTING);
      gsn_resistance->improve( ch, false );
    }

}


/*
 * 'truesight' skill command
 */

SKILL_RUNP( truesight )
{
    int mana = gsn_truesight->getMana( );

  if (!gsn_truesight->available( ch ))
  {
    ch->println( "Ась?" );
    return;
  }

  if (!gsn_truesight->usable( ch ))
    return;

  if (ch->isAffected(gsn_truesight))
    {
      ch->println( "Твои глаза настолько зорки, насколько это возможно." );
      return;
    }

  if (ch->mana < mana)
    {
      ch->println( "У тебя не хватает энергии для этого." );
      return;
    }

  ch->setWait( gsn_truesight->getBeats( )  );

  if (!ch->is_npc() && number_percent() < gsn_truesight->getEffective( ch ))
    {
      Affect af;

      af.where  = TO_DETECTS;
      af.type         = gsn_truesight;
      af.level         = ch->getModifyLevel();
      af.duration = ch->getModifyLevel() / 2 + 5;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = DETECT_HIDDEN;
      affect_to_char(ch, &af);

      af.bitvector = DETECT_INVIS;
      affect_to_char(ch, &af);

      af.bitvector = DETECT_IMP_INVIS;
      affect_to_char(ch,&af);

// Нечего ! Корвин.
//      af.bitvector = ACUTE_VISION;
//      affect_to_char(ch,&af);

      af.bitvector = DETECT_MAGIC;
      affect_to_char(ch,&af);

      ch->mana -= mana; 

      act_p("Ты зорко смотришь вокруг!",ch,0,0,TO_CHAR,POS_RESTING);
      act_p("$c1 смотрит более зорко.",ch,0,0,TO_ROOM,POS_RESTING);
      gsn_truesight->improve( ch, true );
    }
  else
    {
      ch->mana -= mana / 2;

     ch->println( "Ты зорко смотришь вокруг, но не видишь ничего нового." );
      act_p("$c1 зорко смотрит вокруг, но ничего нового не замечает.",
             ch,0,0,TO_ROOM,POS_RESTING);
      gsn_truesight->improve( ch, false );
    }

}


/*
 * 'bandage' skill command
 */

SKILL_RUNP( bandage )
{
        int heal;

        if ( !gsn_bandage->usable( ch ) )
                return;

        if ( gsn_bandage->getEffective( ch ) == 0)
        {
                ch->println( "Что?" );
                return;
        }

        if (ch->isAffected(gsn_bandage))
        {
                act_p("Ты уже перевяза$gло|л|ла свои раны!",ch,0,0,TO_CHAR,POS_RESTING);
                return;
        }

        if (SHADOW(ch))
        {
                ch->println( "Как это наверное интересно смотрится со стороны - бинтовать собственную тень." );
                act_p("$c1 пытается забинтовать свою собственную тень\n\r...похоже кому-то нужен доктор.",
                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }

        int skill = ch->is_npc() ? 100 : gsn_bandage->getEffective( ch );
        if ( number_percent() < skill )
        {
                Affect af;

                ch->setWaitViolence( 1 );

                ch->println( "Ты накладываешь повязку на свою рану!" );
                act_p("$c1 перевязывает свои раны.",ch,0,0,TO_ROOM,POS_RESTING);
                gsn_bandage->improve( ch, true );

                heal = ch->applyCurse( dice(4, 8 ) + ch->getModifyLevel() / 2 );
                ch->hit = min( ch->hit + heal, (int)ch->max_hit );
                update_pos( ch );
                ch->println( "Тебе становится лучше!" );

                af.where        = TO_AFFECTS;
                af.type                = gsn_bandage;
                af.level        = ch->getModifyLevel();
                af.duration        = ch->getModifyLevel() / 10;
                af.modifier        = ch->applyCurse( min( 15, ch->getModifyLevel( ) / 2 ) );
                af.bitvector         = AFF_REGENERATION;
                af.location        = 0;
                affect_to_char(ch,&af);
        }
        else
        {
                ch->setWaitViolence( 1 );

                ch->println( "Ты пытаешься перевязать свои раны, но пальцы не слушаются тебя." );
                gsn_bandage->improve( ch, false );
        }
}


/*--------------------------------------------------------------------------
 * Wiseman 
 *-------------------------------------------------------------------------*/
ClanHealerBattlerager::ClanHealerBattlerager( ) : healPets( false )
{
}

void ClanHealerBattlerager::speech( Character *wch, const char *speech )
{
    if (!speech[0] || str_cmp( speech, "aid me wiseman" ))
        return;
    
    if ((wch->is_npc( ) && (!wch->master 
                            || wch->master->getClan( ) != clan
                            || !healPets))
        || (!wch->is_npc( ) && wch->getClan( ) != clan)) 
    {
        do_say(ch, "Я не хочу помогать тебе.");
        return;
    }

    if (!IS_AFFECTED(wch,AFF_BLIND) && !IS_AFFECTED(wch,AFF_PLAGUE)
         && !IS_AFFECTED(wch,AFF_POISON) && !IS_AFFECTED(wch,AFF_CURSE) )
    {
        do_say(ch, "Ты не нуждаешься в моей помощи.");
        return;
    }

    act_p("$c1 дает тебе лечебное зелье, предлагая сьесть его.",
           ch,0,wch,TO_VICT,POS_RESTING);
    act_p("Ты съедаешь лечебное зелье.",ch,0,wch,TO_VICT,POS_RESTING);
    act_p("Ты передаешь лечебное зелье $C3.",ch,0,wch,TO_CHAR,POS_RESTING);
    act_p("$C1 съедает лечебное зелье, данное тобой.",ch,0,wch,TO_CHAR,POS_RESTING);
    act_p("$c1 дает лечебное зелье $C3.",ch,0,wch,TO_NOTVICT,POS_RESTING);
    act_p("$C1 съедает лечебное зелье, которое $m да$gло|л|ла $c1.",ch,0,wch,TO_NOTVICT,POS_RESTING);

    wch->is_npc( ) ? wch->master->setWaitViolence( 1 ) : wch->setWaitViolence( 1 );

    if (IS_AFFECTED(wch,AFF_BLIND))
        ::spell( gsn_cure_blindness, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );

    if (IS_AFFECTED(wch,AFF_PLAGUE))
        ::spell( gsn_cure_disease, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );

    if (IS_AFFECTED(wch,AFF_POISON))
        ::spell( gsn_cure_poison, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );

    if (IS_AFFECTED(wch,AFF_CURSE))
        ::spell( gsn_remove_curse, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );
}

/*--------------------------------------------------------------------------
 * Powerman 
 *-------------------------------------------------------------------------*/
bool ClanGuardBattlerager::specFight( )
{
    Character *victim;

    if ( !ch->isAffected(gsn_spellbane) )
            interpret( ch, "spellbane" );

    if (!( victim = getVictim( ) ))
        return true;

    if ( number_percent() < 33 )
    {
            act("Ты наносишь тройной удар смертоносной силы!",ch,0,0,TO_CHAR);
            act("$c1 наносит тройной удар смертоносной силы!",ch,0,0,TO_ROOM);
            one_hit( ch, victim );
            one_hit( ch, victim );
            one_hit( ch, victim );
    }

    if ( !ch->isAffected(gsn_resistance) )
            interpret( ch, "resistance" );

    if ( ch->hit < (ch->max_hit /3) && !IS_AFFECTED(ch, AFF_REGENERATION) )
            interpret( ch, "bandage" );

    return true;
}

void ClanGuardBattlerager::actGreet( PCharacter *wch )
{
    do_say(ch, "Добро пожаловать, великий Воин.");
}

void ClanGuardBattlerager::actPush( PCharacter *wch )
{
    act( "$C1 отвешивает тебе нехилый подзатыльник...", wch, 0, ch, TO_CHAR );
    act( "$C1 отвешивает $c3 подзатыльник...\n\r$c1 - как ветром сдуло.", wch, 0, ch, TO_ROOM );
}



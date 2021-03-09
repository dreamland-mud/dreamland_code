
/* $Id: group_necromancy.cpp,v 1.1.2.16.4.7 2009/11/05 03:18:39 rufina Exp $
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

#include "group_necromancy.h"
#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "interp.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

#define MOB_VNUM_UNDEAD               18

CLAN(none);
PROF(necromancer);

NecroCreature::~NecroCreature( )
{
}

void AdamantiteGolem::fight( Character *victim )
{
    Character *master, *rch;
    int mirrors;

    BasicMobileDestiny::fight( victim );        
    
    master = ch->master;

    if (!master
        || !master->fighting
        || master->is_npc( ) )
        return;
    
    if (master->fighting->fighting != master)
        return;

    if (ch->fighting == master)
        return;
    
    if (!ch->can_see( master ))
        return;

    if (is_safe( ch, master->fighting ) || is_safe( ch, master ))
        return;

    for (mirrors = 0, rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch->is_mirror( ) && rch->doppel == master)
            mirrors++;
    
    if (number_percent( ) > 100 - mirrors * 5)
        return;

    ch->setWait( gsn_rescue->getBeats( )  );
    
    oldact("Ты спасаешь $C4!",  ch, 0, master, TO_CHAR );
    oldact("$c1 спасает тебя!", ch, 0, master, TO_VICT );
    oldact("$c1 спасает $C4!",  ch, 0, master, TO_NOTVICT );

    stop_fighting( master->fighting, false );
    set_fighting( ch, master->fighting);
    set_fighting( master->fighting, ch );
}

SPELL_DECL_T(AdamantiteGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, AdamantiteGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
                                     10 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 4000,
                                     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                                     13, 9, ch->getModifyLevel( ) / 2 + 10 );
    
    for (int i = 0; i < stat_table.size; i ++)
        mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob;
}


SPELL_DECL_T(IronGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, IronGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
                                     10 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 1000,
                                     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                                     11, 5, ch->getModifyLevel( ) / 2 + 10 );
    
    for (int i = 0; i < stat_table.size; i ++)
        mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob;
}


SPELL_DECL_T(LesserGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, LesserGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
                                     2 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 400,
                                     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                                     3, 10, ch->getModifyLevel( ) / 2 );
    
    for (int i = 0; i < stat_table.size; i ++)
        mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob; 
}

SPELL_DECL_T(StoneGolem, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, StoneGolem)::createMobile( Character *ch, int level ) const 
{
    NPCharacter *mob = createMobileAux( ch, ch->getModifyLevel( ), 
                                     5 * (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit) + 2000,
                                     (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                                     8, 4, ch->getModifyLevel( ) / 2 );
    
    for (int i = 0; i < stat_table.size; i ++)
        mob->perm_stat[i] = min( 25, 15 + ch->getModifyLevel( ) / 10 );

    mob->perm_stat[STAT_STR] += 3;
    mob->perm_stat[STAT_INT] -= 1;
    mob->perm_stat[STAT_CON] += 2;
    return mob;
}


SPELL_DECL_T(SummonShadow, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, SummonShadow)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, ch->getModifyLevel( ), 
                         ch->max_hit, ch->max_mana,
                         number_range(level/15, level/10),
                         number_range(level/3, level/2),
                         number_range(level/8, level/6) );
}

SPELL_DECL(AnimateDead);
VOID_SPELL(AnimateDead)::run( Character *ch, Object *obj, int sn, int level ) 
{
        NPCharacter *undead;
        Object *obj2,*next;
        MOB_INDEX_DATA *pCorpseOwner = 0;

        if ( !(obj->item_type == ITEM_CORPSE_NPC
                        || obj->item_type == ITEM_CORPSE_PC))
        {
                ch->pecho("Это заклинание работает только на трупы.");
                return;
        }

        if ( !ch->is_immortal() && obj->item_type == ITEM_CORPSE_PC )
        {
                ch->pecho("Некромантия с трупами игроков запрещена Богами.");
                return;
        }

        if ( ch->isAffected(sn ) )
        {
                ch->pecho("Нужно восстановить энергию после предыдущего оживления.");
                return;
        }

        if ( overcharmed( ch ) )
                return;

        if ( ch->in_room && IS_SET( ch->in_room->room_flags, ROOM_NO_MOB ) )
        {
                ch->pecho("В этом месте могут находиться только игроки.");
                return;
        }

        if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE )
                || IS_SET(ch->in_room->room_flags, ROOM_PRIVATE )
                || IS_SET(ch->in_room->room_flags, ROOM_SOLITARY ) )
        {
                ch->pecho("Святость этого места не позволяет тебе сделать этого.");
                return;
        }

        if (obj->value3()) 
            pCorpseOwner = get_mob_index( obj->value3() );

        if (!pCorpseOwner || DLString(pCorpseOwner->short_descr).empty()) {
            ch->pecho("Этот труп не поддается восстановлению.");
            return;
        }

        undead = create_mobile( get_mob_index(MOB_VNUM_UNDEAD) );

        for (int i=0; i < stat_table.size; i++ )
        {
                undead->perm_stat[i] = min(25,2 * ch->perm_stat[i]);
        }

        undead->max_hit = ch->is_npc() ? ch->max_hit : ch->getPC( )->perm_hit;
        undead->hit = undead->max_hit;
        undead->max_mana = ch->is_npc() ? ch->max_mana : ch->getPC( )->perm_mana;
        undead->mana = undead->max_mana;
        undead->alignment = ch->alignment;
        undead->setLevel( min(100, ( ch->getModifyLevel() - 2 ) ) );

        for (int i=0; i < 3; i++ )
                undead->armor[i] = interpolate(undead->getRealLevel( ),100,-100);
        undead->armor[3] = interpolate(undead->getRealLevel( ),50,-200);
        undead->gold = 0;

        SET_BIT(undead->act, ACT_UNDEAD);
        SET_BIT(undead->affected_by, AFF_CHARM);
        SET_BIT(undead->form, FORM_INSTANT_DECAY);
        undead->timer = (undead->getRealLevel( ) / 10 + 1) * 60 * 24; // 1 day per 10lev 
        undead->master = ch;
        undead->leader = ch;
        
        if (pCorpseOwner->sex != SEX_EITHER) 
            undead->setSex( pCorpseOwner->sex );
        else
            undead->setSex( ch->getSex( ) );

        // Replace "%s" in the short and long descrs of the undead with original mob name.
        DLString whose = russian_case(pCorpseOwner->short_descr, '2');
        undead->fmtName(undead->getName().c_str(), pCorpseOwner->player_name);
        undead->fmtShortDescr(undead->getShortDescr(), whose.c_str());
        undead->fmtLongDescr(undead->getLongDescr(), whose.c_str());

        char_to_room(undead,ch->in_room);

        for ( obj2 = obj->contains;obj2;obj2=next )
        {
                next = obj2->next_content;
                obj_from_obj(obj2);
                obj_to_char(obj2, undead);
        }
        interpret_raw( undead,"wear", "all" );

        postaffect_to_char( ch, sn, ch->getModifyLevel() / 10 );

        ch->pecho("С помощью сил Тьмы и Хаоса ты оживляешь труп!");
        ch->recho("С помощью сил Тьмы и Хаоса %C1 оживляет %O4!", ch, obj);
        oldact("$C1 смотрит на тебя бессмысленным взглядом, повинуясь твоим приказам!",ch,0,undead,TO_CHAR);

        extract_obj (obj);
}

VOID_SPELL(AnimateDead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if ( victim == ch )
            ch->pecho("Жизнь прекрасна и ты не собираешься расставаться со своим телом!");
        else 
            oldact("Жизнь прекрасна и $C1 не собирается расставаться со своим телом!",ch,0,victim,TO_CHAR);

}


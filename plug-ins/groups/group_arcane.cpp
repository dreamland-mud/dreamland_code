/* $Id: objcast.cpp,v 1.1.2.17.4.13 2010-09-01 21:20:45 rufina Exp $
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
#include "skill.h"
#include "skillcommandtemplate.h"
#include "spelltarget.h"
#include "defaultspell.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"

#include "dreamland.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "clanreference.h"
#include "vnum.h"
#include "merc.h"
#include "../anatolia/handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

CLAN(battlerager);
GSN(scrolls);
GSN(staves);
GSN(wands);
GSN(none);

/*
 * 'recite' skill command
 */
static void recite_one_spell(Character *ch, Object *scroll, Spell::Pointer &spell, const DLString &args,
                             int &successfulSpells, int &successfulTargets)
{
    ostringstream errBuf;
    SpellTarget::Pointer t = spell->locateTargets( ch, args, errBuf );
    if (t->castFar && t->door != -1) {
        ch->send_to( "На таком расстоянии жертва ничего не почувствует.\r\n" );
        return;
    }

    if (t->error != 0) {
        switch (t->error) {
        case TARGET_ERR_CAST_ON_WHOM:
            ch->pecho("Ты зачитываешь одно из заклинаний с %O2, но оно не находит, на кого подействовать.", scroll);
            break;
        case TARGET_ERR_CAST_ON_WHAT:
            ch->pecho("Ты зачитываешь одно из заклинаний с %O2, но оно не находит, на что подействовать.", scroll);
            break;
        default:
            ch->send_to(errBuf);
            break;
        }

        return;
    }

    act_p( "$c1 зачитывает заклинание с $o2.", ch, scroll, 0, TO_ROOM,POS_RESTING );
    act_p( "Ты зачитываешь одно из заклинаний с $o2.", ch, scroll, 0, TO_CHAR,POS_RESTING );

    successfulTargets++;

    if (number_percent( ) >= gsn_scrolls->getEffective( ch )) {
        act("Ты не совлада$gло|л|ла с произношением.", ch, 0, 0, TO_CHAR);
        gsn_scrolls->improve( ch, false );
        return;
    }

    gsn_scrolls->improve( ch, true );

    try {
        bitstring_t recite_flags = FSPELL_BANE|FSPELL_CHECK_SAFE|FSPELL_ATTACK_CASTER;

        int slevel;
        slevel = scroll->value0() + skill_level_bonus(*gsn_scrolls, ch);
      
        if (::spell_nocatch(spell, slevel, ch, t, recite_flags)) {
            successfulSpells++;
        }

        if (ch->is_adrenalined() || ch->fighting != 0) 
            ch->setWait(4);        

    } catch (const VictimDeathException &vde) {
        return;
    }
}

SKILL_RUNP( recite )
{
    Object *scroll;
    DLString args = argument, arg1;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->send_to("Какие еще свитки-шмитки?! Ты же вои{Smн{Sfтельница{Sx клана Ярости, а не презренный МАГ!\n\r");
        return;
    }

    arg1 = args.getOneArgument( );

    if (( scroll = get_obj_carry( ch, arg1.c_str( ) ) ) == 0) {
        ch->send_to("У тебя нет такого свитка.\n\r");
        return;
    }

    if (scroll->item_type != ITEM_SCROLL) {
        ch->send_to("С помощью этой команды можно зачитывать только свитки.\n\r");
        return;
    }

    if (get_wear_level( ch, scroll ) > ch->getRealLevel()) {
        ch->send_to("Этот свиток чересчур сложен для твоего понимания.\n\r");
        return;
    }
    
    int successfulSpells = 0, successfulTargets = 0;
    int totalSpells = 0;

    for (int i = 1; i <= 4; i++) {
        int sn = scroll->valueByIndex(i);
        if (sn < 0)
            continue;

        Skill *skill = SkillManager::getThis( )->find( sn );
        if (!skill || skill->getIndex() == gsn_none)
            continue;

        Spell::Pointer spell = skill->getSpell( );
        if (!spell) {
            LogStream::sendWarning() << "Scroll [" << scroll->pIndexData->vnum << "] has invalid spell for " << i << endl;
            continue;
        }

        totalSpells++;
        recite_one_spell(ch, scroll, spell, args, successfulSpells, successfulTargets);
    }

    if (successfulSpells > 0) {
        ch->pecho("%^O1 превращается в пыль.", scroll);
        extract_obj( scroll );
    }
}



/*
 * 'brandish' skill command
 */
SKILL_RUNP( brandish )
{
    Object *staff;
    int sn;
    Skill::Pointer skill;
    Spell::Pointer spell;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->send_to("Палками махать?! Ты же вои{Smн{Sfтельница{Sx клана Ярости, а не презренный МАГ!\n\r");
        return;
    }

    if (( staff = get_eq_char( ch, wear_hold ) ) == 0) {
        ch->send_to("Чтобы пользоваться посохами, их надо взять в руки.\n\r");
        return;
    }

    if (staff->item_type != ITEM_STAFF) {
        ch->send_to("Ты можешь взмахнуть только посохом.\n\r");
        return;
    }
    
    sn = staff->value3();
    skill = SkillManager::getThis( )->find( sn );
    
    if (!skill || !( spell = skill->getSpell( ) )) { 
        bug( "brandish: bad sn %d.", sn );
        return;
    }

     ch->setWaitViolence( 2 );

    if (staff->value2() > 0) {
        const char *terrain = terrains[ch->in_room->getSectorType()].hit;
        
        act( "$c1 ударяет $o5 $T.", ch, staff, terrain, TO_ROOM );
        act( "Ты ударяешь $o5 $T.", ch, staff, terrain, TO_CHAR );

        if ( number_percent( ) >= gsn_staves->getEffective( ch ))
        {
            act_p("Ты не смо$gгло|г|гла активировать $o4.",ch,staff,0,TO_CHAR,POS_RESTING);
            act_p("...и ничего не происходит.",ch,0,0,TO_ROOM,POS_RESTING);
            gsn_staves->improve( ch, false );
        }
        else {
            Character *vch, *vch_next;
            bool offensive;
            int level, t;
            
            level = staff->value0() + skill_level_bonus(*gsn_staves, ch);
            offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;
            t = spell->getTarget( );            

            try {
                if (IS_SET( t, TAR_IGNORE|TAR_CREATE_MOB|TAR_CREATE_OBJ )) {
                    ::spell_nocatch(spell, level, ch, SpellTarget::Pointer(NEW, str_empty), 0);
                    gsn_staves->improve( ch, true );
                }
                else if (IS_SET( t, TAR_ROOM|TAR_PEOPLE )) {
                    ::spell_nocatch(spell, level, ch, SpellTarget::Pointer(NEW, ch->in_room), 0);
                    gsn_staves->improve( ch, true );
                }
                else if (IS_SET( t, TAR_CHAR_SELF )) {
                    ::spell_nocatch(spell, level, ch, SpellTarget::Pointer(NEW, ch), 0);
                    gsn_staves->improve( ch, true, ch );
                }
                else if (IS_SET( t, TAR_CHAR_ROOM )) {
                    bitstring_t brandish_flags = 
                        FSPELL_BANE|FSPELL_ATTACK_CASTER|FSPELL_CHECK_GROUP|FSPELL_CHECK_SAFE;

                    for (vch = ch->in_room->people; vch; vch = vch_next) {
                        vch_next = vch->next_in_room;

                        if (::spell_nocatch(spell, level, ch, SpellTarget::Pointer(NEW, vch), brandish_flags))
                            if (offensive)
                                yell_panic( ch, vch,
                                            "Помогите! Кто-то размахивает около меня посохом!",
                                            "Помогите! %1$^C1 пугает меня посохом!" );

                        gsn_staves->improve( ch, true, vch );
                    }
                }
                else {
                    bug( "brandish: bad target for sn %d.", sn );
                    return;
                }
            } catch (const VictimDeathException &e) {
            }
        }
    }

    staff->value2(staff->value2() - 1);
    if (staff->value2() <= 0) {
        ch->recho( "%1$^O1 %2$C2 темне%1$nет|ют и исчеза%1$nет|ют.", staff, ch );
        ch->pecho( "Тво%1$Gе|й|я|и %1$O1 темне%1$nет|ют и исчеза%1$nет|ют.", staff );
        extract_obj( staff );
    }
}


/*
 * 'zap' skill command
 */
SKILL_RUNP( zap )
{
    std::basic_ostringstream<char> buf;
    Object *wand;
    int sn;
    Skill::Pointer skill;
    Spell::Pointer spell;
    SpellTarget::Pointer target;
    DLString args = argument;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->pecho("Ты долж%1$Gно|ен|на уничтожать магию, а не использовать её!", ch);
        return;
    }

    if (( wand = get_eq_char( ch, wear_hold ) ) == 0) {
        ch->send_to("Жезл нужно сначала взять в руку.\n\r");
        return;
    }

    if (wand->item_type != ITEM_WAND) {
        ch->send_to("Ты можешь взмахнуть только волшебным жезлом!\n\r");
        return;
    }
    
    sn = wand->value3();
    skill = SkillManager::getThis( )->find( sn );

    if (!skill || !( spell = skill->getSpell( ) )) {
        bug( "zap: bad sn %d", sn );
        return;
    }

    target = spell->locateTargets( ch, args, buf );

    if (target->error != 0) {
        switch (target->error) {
        case TARGET_ERR_CAST_ON_WHAT:
            ch->println("Взмахнуть жезлом на что?");
            break;
        case TARGET_ERR_CAST_ON_WHOM:
            ch->println("Взмахнуть жезлом на кого именно?");
            break;
        default:
            ch->send_to(buf);
            break;
        }

        return;
    }
    
     ch->setWaitViolence( 2 );
    
    if (target->castFar && target->door != -1) {
        ch->send_to( "Твой жезл не дотягивается до соседней комнаты.\r\n" );
        return;
    }
    
    if (wand->value2() > 0) {
        Character *victim = target->victim;

        if (victim && victim->in_room == ch->in_room) {
            if (ch != victim) {
                act( "$c1 взмахивает $o5 на тебя!", ch, wand, victim, TO_VICT );
                act( "$c1 взмахивает $o5 на $C4.", ch, wand, victim, TO_NOTVICT );
                act( "Ты взмахиваешь $o5 на $C4.", ch, wand, victim, TO_CHAR );
            }
            else {
                act( "$c1 взмахивает $o5 на себя.", ch, wand, 0, TO_ROOM );
                act( "Ты взмахиваешь $o5 на себя.", ch, wand, 0, TO_CHAR );
            }
        }
        else if (target->obj && target->obj->getRoom( ) == ch->in_room) {
            act( "$c1 взмахивает $o5 на $O4.", ch, wand, target->obj, TO_ROOM );
            act( "Ты взмахиваешь $o5 на $O4.", ch, wand, target->obj, TO_CHAR );
        }
        else {
            act( "$c1 взмахивает $o5.", ch, wand, 0, TO_ROOM );
            act( "Ты взмахиваешь $o5.", ch, wand, 0, TO_CHAR );
        }

        if (number_percent() >= gsn_wands->getEffective( ch )) {
            act_p( "Твои усиленные манипуляции с $o5 приводят лишь к дыму и искрам.",
                    ch,wand,0,TO_CHAR,POS_RESTING);
            act_p( "Усиленные манипуляции $c2 с $o5 приводят лишь к дыму и искрам.",
                    ch,wand,0,TO_ROOM,POS_RESTING);
            gsn_wands->improve( ch, false, victim );
        }
        else {
            try {
                bitstring_t zap_flags = FSPELL_CHECK_SAFE|FSPELL_BANE|FSPELL_ATTACK_CASTER;
                
                int slevel;
                slevel = wand->value0() + skill_level_bonus(*gsn_wands, ch);
              
                if (::spell_nocatch(spell, wand->value0(), ch, target, zap_flags))
                    if (spell->getSpellType( ) == SPELL_OFFENSIVE)
                        yell_panic( ch, victim,
                                    "Помогите! Кто-то размахивает около меня волшебным жезлом!",
                                    "Помогите! %1$^C1 пугает меня своим жезлом!" );

            } catch (const VictimDeathException &e) {
            }

            gsn_wands->improve( ch, true, victim );
        }
    }

    wand->value2(wand->value2() - 1);
    if (wand->value2() <= 0) {
        ch->recho( "%1$^O1 %2$C2 развалива%1$nется|ются на куски.", wand, ch );
        ch->pecho( "Тво%1$Gе|й|я|и %1$O1 развалива%1$nется|ются на куски.", wand );
        extract_obj( wand );
    }
}

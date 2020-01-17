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
#include <vector>

#include "skillreference.h"
#include "skill.h"
#include "spell.h"
#include "spelltarget.h"
#include "skillmanager.h"
#include "commandtemplate.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"

#include "dreamland.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "fight_exception.h"
#include "clanreference.h"
#include "vnum.h"
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

CLAN(battlerager);
GSN(scrolls);
GSN(staves);
GSN(wands);
short get_wear_level( Character *ch, Object *obj );

static bool oprog_quaff( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Quaff", "C", ch );
    FENIA_NDX_CALL( obj, "Quaff", "OC", obj, ch );
    return false;
}

/*
 * 'quaff' skill command
 */
CMDRUNP( quaff )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;

    one_argument( argument, arg );

    if(!ch->is_npc( ) && ch->getClan( ) == clan_battlerager && !ch->is_immortal( )) {
        ch->send_to("Ты же BattleRager, а не презренный МАГ!\n\r");
        return;
    }

    if (arg[0] == '\0') {
        ch->send_to("Осушить что?\n\r");
        return;
    }

    if (( obj = get_obj_carry( ch, arg ) ) == 0) {
        ch->send_to("У тебя нет такого снадобья.\n\r");
        return;
    }

    if (obj->item_type != ITEM_POTION) {
        ch->send_to("Ты можешь осушить только снадобья.\n\r");
        return;
    }

    if (ch->getModifyLevel( ) < obj->level) {
        ch->pecho("Эта смесь чересчур сильна, чтобы ты мог%1$Gло||ла выпить ее.", ch);
        return;
    }

    act_p( "$c1 осушает $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
    act_p( "Ты осушаешь $o4.", ch, obj, 0 ,TO_CHAR,POS_RESTING );
    
    if (oprog_quaff( obj, ch ))
        return;

    spell_by_item( ch, obj );

    if (ch->is_adrenalined( ) || ch->fighting)
         ch->setWaitViolence( 2 );
    
    extract_obj( obj );
    obj_to_char( create_object(get_obj_index(OBJ_VNUM_POTION_VIAL),0),ch);
}



/*
 * 'recite' skill command
 */

CMDRUNP( recite )
{
    std::basic_ostringstream<char> buf;
    std::vector<SpellTarget::Pointer> targets;
    SpellTarget::Pointer t;
    Spell::Pointer spell;
    Object *scroll;
    bool found;
    DLString args = argument, arg1;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->send_to("RECITE?!  Ты же BattleRager, а не презренный МАГ!\n\r");
        return;
    }

    arg1 = args.getOneArgument( );

    if (( scroll = get_obj_carry( ch, arg1.c_str( ) ) ) == 0) {
        ch->send_to("У тебя нет такого свитка.\n\r");
        return;
    }

    if (scroll->item_type != ITEM_SCROLL) {
        ch->send_to("Ты можешь зачитать только свиток.\n\r");
        return;
    }

    if (get_wear_level( ch, scroll ) > ch->getModifyLevel( )) {
        ch->send_to("Этот свиток чересчур сложен для твоего понимания.\n\r");
        return;
    }
    
    found = false;

    for (int i = 1; i <= 4; i++) {
        int sn = scroll->value[i];
        
        if (sn > 0) {
            spell = SkillManager::getThis( )->find( sn )->getSpell( );

            if (spell) { 
                t = spell->locateTargets( ch, args, buf );

                if (t) {
                    if (t->castFar && t->door != -1) {
                        ch->send_to( "На таком расстоянии жертва ничего не почувствует.\r\n" );
                        return;
                    }

                    targets.push_back( t );
                    found = true;
                    continue;
                }
            }
        }

        targets.push_back( SpellTarget::Pointer( ) );
    }
    
    if (!found) {
        ch->send_to( "На кого или на что ты хочешь зачитать этот свиток?\r\n" );
        return;
    }
    
    act_p( "$c1 зачитывает $o4.", ch, scroll, 0, TO_ROOM,POS_RESTING );
    act_p( "Ты зачитываешь $o4.", ch, scroll, 0, TO_CHAR,POS_RESTING );

    if (number_percent( ) >= gsn_scrolls->getEffective( ch )) {
        act("Ты не совлада$gло|л|ла с произношением.", ch, 0, 0, TO_CHAR);
        gsn_scrolls->improve( ch, false );
    }
    else {
        bool offensive;
        
        for (int i = 1; i <= 4; i++) {
            t = targets[i - 1];

            if (!t)
                continue;
            
            spell = SkillManager::getThis( )->find( scroll->value[i] )->getSpell( );
            offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;

            if (offensive && t->victim && is_safe( ch, t->victim ))
                continue;
            
            if (!spell->spellbane( ch, t->victim )) {
                try {
                    spell->run( ch, t, scroll->value[0] );

                    if (offensive)
                        attack_caster( ch, t->victim );
                } catch (const VictimDeathException &e) {
                    break;
                }
            }
        }
        
        gsn_scrolls->improve( ch, true );

        if (ch->is_adrenalined() || ch->fighting != 0) 
             ch->setWaitViolence( 2 );
    }

    extract_obj( scroll );
}



/*
 * 'brandish' skill command
 */

CMDRUNP( brandish )
{
    Object *staff;
    int sn;
    Skill::Pointer skill;
    Spell::Pointer spell;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->send_to("Ты же BattleRager, а не презренный МАГ!\n\r");
        return;
    }

    if (( staff = get_eq_char( ch, wear_hold ) ) == 0) {
        ch->send_to("Ты не держишь ничего в руке.\n\r");
        return;
    }

    if (staff->item_type != ITEM_STAFF) {
        ch->send_to("Ты можешь взмахнуть только посохом.\n\r");
        return;
    }
    
    sn = staff->value[3];
    skill = SkillManager::getThis( )->find( sn );
    
    if (!skill || !( spell = skill->getSpell( ) )) { 
        bug( "brandish: bad sn %d.", sn );
        return;
    }

     ch->setWaitViolence( 2 );

    if (staff->value[2] > 0) {
        const char *terrain = terrains[ch->in_room->sector_type].hit;
        
        act( "$c1 ударяет $o5 $T.", ch, staff, terrain, TO_ROOM );
        act( "Ты ударяешь $o5 $T.", ch, staff, terrain, TO_CHAR );

        if (ch->getModifyLevel( ) + 3 < staff->level
            || number_percent( ) >= gsn_staves->getEffective( ch ))
        {
            act_p("Ты не смо$gгло|г|гла активировать $o4.",ch,staff,0,TO_CHAR,POS_RESTING);
            act_p("...и ничего не происходит.",ch,0,0,TO_ROOM,POS_RESTING);
            gsn_staves->improve( ch, false );
        }
        else {
            Character *vch, *vch_next;
            bool offensive;
            int level, t;
            
            level = staff->value[0];
            offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;
            t = spell->getTarget( );

            try {
                if (IS_SET( t, TAR_IGNORE|TAR_CREATE_MOB|TAR_CREATE_OBJ )) {
                    spell->run( ch, str_empty, sn, level );
                    gsn_staves->improve( ch, true );
                }
                else if (IS_SET( t, TAR_ROOM|TAR_PEOPLE )) {
                    spell->run( ch, ch->in_room, sn, level );
                    gsn_staves->improve( ch, true );
                }
                else if (IS_SET( t, TAR_CHAR_SELF )) {
                    spell->run( ch, ch, sn, level );
                    gsn_staves->improve( ch, true, ch );
                }
                else if (IS_SET( t, TAR_CHAR_ROOM )) {
                    for (vch = ch->in_room->people; vch; vch = vch_next) {
                        vch_next = vch->next_in_room;
                        
                        if (offensive) {
                            if (is_safe( ch, vch ) || is_same_group( ch, vch ))
                                continue;

                            if (!spell->spellbane( ch, vch )) {
                                spell->run( ch, vch, sn, level );
                                attack_caster( ch, vch );
                            }
                            
                            yell_panic( ch, vch,
                                        "Помогите! Кто-то размахивает около меня посохом!",
                                        "Помогите! %1$^C1 пугает меня посохом!" );
                        }
                        else {
                            if (!spell->spellbane( ch, vch ))
                                spell->run( ch, vch, sn, level );
                        }

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

    if (--staff->value[2] <= 0) {
        ch->recho( "%1$O1 %2$C2 темне%1$nет|ют и исчеза%1$nет|ют.", staff, ch );
        ch->pecho( "Тво%1$Gе|й|я|и %1$O1 темне%1$nет|ют и исчеза%1$nет|ют.", staff );
        extract_obj( staff );
    }
}


/*
 * 'zap' skill command
 */

CMDRUNP( zap )
{
    std::basic_ostringstream<char> buf;
    Object *wand;
    int sn;
    Skill::Pointer skill;
    Spell::Pointer spell;
    SpellTarget::Pointer target;
    DLString args = argument;

    if (!ch->is_npc( ) && ch->getClan( ) == clan_battlerager) {
        ch->send_to("Ты должен уничтожать магию, а не использовать её!\n\r");
        return;
    }

    if (( wand = get_eq_char( ch, wear_hold ) ) == 0) {
        ch->send_to("Но ведь в твоих руках ничего нет!\n\r");
        return;
    }

    if (wand->item_type != ITEM_WAND) {
        ch->send_to("Ты можешь взмахнуть только волшебным жезлом!\n\r");
        return;
    }
    
    sn = wand->value[3];
    skill = SkillManager::getThis( )->find( sn );

    if (!skill || !( spell = skill->getSpell( ) )) {
        bug( "zap: bad sn %d", sn );
        return;
    }

    target = spell->locateTargets( ch, args, buf );

    if (!target) {
        ch->send_to("Кого или что ты хочешь поразить?\n\r");
        return;
    }
    
     ch->setWaitViolence( 2 );
    
    if (target->castFar && target->door != -1) {
        ch->send_to( "Твой жезл не дотягивается до соседней комнаты.\r\n" );
        return;
    }
    
    if (wand->value[2] > 0) {
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
            bool offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;
                
            if (offensive && victim && is_safe( ch, victim ))
                return;

            try {
                if (!spell->spellbane( ch, victim )) {
                    spell->run( ch, target, wand->value[0] );

                    if (offensive)
                        attack_caster( ch, victim );
                }
                
                if (offensive)
                    yell_panic( ch, victim,
                                "Помогите! Кто-то размахивает около меня волшебным жезлом!",
                                "Помогите! %1$^C1 пугает меня своим жезлом!" );

            } catch (const VictimDeathException &e) {
            }

            gsn_wands->improve( ch, true, victim );
        }
    }

    if (--wand->value[2] <= 0) {
        ch->recho( "%1$O1 %2$C2 развалива%1$nется|ются на куски.", wand, ch );
        ch->pecho( "Тво%1$Gе|й|я|и %1$O1 развалива%1$nется|ются на куски.", wand );
        extract_obj( wand );
    }
}

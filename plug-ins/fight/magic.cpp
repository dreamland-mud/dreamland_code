/* $Id: magic.cpp,v 1.1.2.12 2008/05/27 21:30:02 rufina Exp $
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
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "core/behavior/behavior_utils.h"
#include "logstream.h"
#include "spell.h"
#include "spelltarget.h"
#include "skillmanager.h"
#include "skill.h"
#include "affecthandler.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "room.h"
#include "roomutils.h"
#include "dreamland.h"
#include "merc.h"

#include "act.h"

#include "magic.h"
#include "clanreference.h"

#include "act_move.h"
#include "../anatolia/handler.h"
#include "interp.h"
#include "vnum.h"
#include "fight_exception.h"
#include "immunity.h"
#include "material.h"
#include "fight.h"
#include "def.h"

CLAN(none);
GSN(deafen);
GSN(garble);
GSN(holy_craft);
GSN(shielding);
GSN(spell_craft);

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( short level, Character *victim, int dam_type, Character *ch, bitstring_t dam_flag, bool verbose )
{
    int save;
    int mlevel = victim->getModifyLevel( );

    // average saves on lvl 100: -70, max saves per non-limit item: -5
    // shooting for ~70-80% chance to save with "normal" saves
    // new equation should be:
    // save = (mlevel - level) * 4 - victim->saving_throw / mlevel * 100
    
    save = 40 + (mlevel - level) * 4 -
        (victim->saving_throw * 90) / max( 45, mlevel );
    
    if (IS_AFFECTED(victim,AFF_BERSERK))
        save += mlevel / 5;
    
    switch(immune_check(victim, dam_type, dam_flag)) {
        case RESIST_IMMUNE:
            if (ch && verbose) {
                if (ch != victim)
                    ch->pecho("%^N1, похоже, никак не сможет навредить %C3.", damage_table.message(dam_type).c_str(), victim);
                else
                    ch->pecho("%^N1, похоже, никак не сможет навредить тебе.", damage_table.message(dam_type).c_str());
            }
            return true;
        case RESIST_RESISTANT:
            save += mlevel / 5;
            if (ch && verbose && number_percent( ) < gsn_spell_craft->getEffective( ch )) {
                if (ch != victim)
                    ch->pecho("%^N1 {1{Gочень слабо{2 влияет на %C4.", 
                        damage_table.message(dam_type).c_str(), victim);
                else 
                    ch->pecho("%^N1 {1{Gочень слабо{2 влияет на тебя.", 
                        damage_table.message(dam_type).c_str());
            }
            break;
        case RESIST_VULNERABLE:
            save -= mlevel / 5;
            if (ch && verbose && number_percent( ) < gsn_spell_craft->getEffective( ch )) {
                if (ch != victim)
                    ch->pecho("%^N1 {1{Rособо пагубно{2 влияет на %C4.", 
                        damage_table.message(dam_type).c_str(), victim);
                else
                    ch->pecho("%^N1 {1{Rособо пагубно{2 влияет на тебя.", 
                        damage_table.message(dam_type).c_str());
            }
            break;
    }
    
    if (ch && IS_GOOD(ch) && IS_EVIL(victim))
        if (number_percent( ) < gsn_holy_craft->getEffective( ch )) {
            save -= mlevel / 5;
            gsn_holy_craft->improve( ch, true );
        }

    if (dam_type == DAM_CHARM) {
        int cha_vict = victim->getCurrStat(STAT_CHA);
        int cha_diff = ch ? ch->getCurrStat(STAT_CHA) - cha_vict : 0;
        
        save -= cha_diff * 2;

        if (!victim->is_npc( ) && cha_vict < 18)
            save -= mlevel / 5;
    }
    
    if (ch)
        for (auto &paf: ch->affected.findAllWithHandler())
            if (paf->type->getAffect())
                paf->type->getAffect( )->onSaves(
                    SpellTarget::Pointer(NEW, ch), paf, victim, save, dam_type);

    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

void attack_caster( Character *caster, Character *victim )
{
    if (victim && victim != caster && victim->master != caster) 
        if (victim->in_room == caster->in_room && victim->fighting == 0
            && victim->position > POS_SLEEPING && !victim->isDead( ))
        {
            multi_hit( victim, caster );
        }
}

void area_message( Character *ch, const DLString &msg, bool everywhere )
{
    Character *wch;

    for (wch = char_list; wch; wch = wch->next) {
        if (wch == ch)
            continue;
        
        if (wch->in_room->area != ch->in_room->area)
            continue;

        if (wch->in_room == ch->in_room)
            continue;
            
        if (!IS_AWAKE(wch))
            continue;
            
        if (!everywhere && !RoomUtils::isOutside(wch))
            continue;
        
        wch->pecho( msg );
    }
}

bool mprog_spell( Character *victim, Character *caster, Skill::Pointer skill, bool before )
{
    const char *sname = skill->getName().c_str();
    FENIA_CALL( victim, "Spell", "Csi", caster, sname, before );
    FENIA_NDX_CALL( victim->getNPC( ), "Spell", "CCsi", victim, caster, sname, before );
    BEHAVIOR_CALL( victim->getNPC( ), spell, caster, skill->getIndex(), before );
    return false;
}

void mprog_cast( Character *caster, SpellTarget::Pointer target, Skill::Pointer skill, bool before )
{
    // Call Fenia progs only once.
    if (before == false) {
        const char *sname = skill->getName().c_str();
        for (Character *rch = caster->in_room->people; rch; rch = rch->next_in_room) {
            FENIA_VOID_CALL( rch, "Cast", "Cs", caster, sname );
            FENIA_NDX_VOID_CALL( rch->getNPC( ), "Cast", "CCs", rch, caster, sname );
        }
    }

    BEHAVIOR_VOID_CALL( caster->getNPC( ), cast, target, skill->getIndex(), before );

}

/*
 *
 */
bool spell_nocatch( int sn, int level, Character *ch, SpellTarget::Pointer target, int flags )
{
    Spell::Pointer spell; 
    Skill *skill;

    if (!( skill = SkillManager::getThis( )->find( sn ) ))
        return false;

    if (!( spell = skill->getSpell( ) ))
        return false;

    return spell_nocatch(spell, level, ch, target, flags);
}

bool spell_nocatch( Spell::Pointer &spell, int level, Character *ch, SpellTarget::Pointer target, int flags )
{
    if (!target || !spell)
        return false;

    Skill::Pointer skill = spell->getSkill();

    if (IS_SET(flags, FSPELL_OBSTACLES)) {
        if (ch->isAffected( gsn_shielding ) && number_percent( ) > 50) {
            ch->pecho("Ты пытаешься сотворить заклинание, но изолирующий экран блокирует тебя.");
            ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но изолирующий экран блокирует %1$P2.", ch);
            return false;
        }

        if (ch->isAffected( gsn_garble ) && number_percent( ) > 50) {
            ch->pecho("Твой язык заплетается, и ты не можешь как следует произнести заклинание.");
            ch->recho("Язык %1$C2 заплетается, и %1$P1 не мо%1$nжет|гут как следует произнести заклинание.", ch);        
            return false;
        }
        
        if (ch->isAffected( gsn_deafen ) && number_percent( ) > 50) {
            ch->pecho("Глухота мешает тебе как следует произнести заклинание.");
            ch->recho("Глухота мешает %1$C3 как следует произнести заклинание.", ch);        
            return false;
        }    

        if (target->type == SpellTarget::CHAR
             && target->victim->in_room != ch->in_room
             && !ch->can_see( target->victim ))
            return false;

        if (spell->getPosition() > ch->position)
            return false;
    }

    if (IS_SET(ch->in_room->room_flags, ROOM_NO_CAST)) {
        ch->pecho("Эта местность поглощает и блокирует твое заклинание.");
        ch->recho("%1$^C1 пыта%1$nется|ются сотворить заклинание, но что-то в этой местности блокирует %1$P2.", ch);
        return false;
    }

    if (IS_SET(flags, FSPELL_MANA)) {
        int mana = skill->getMana( ch ) / 2;

        if (ch->mana < mana)
            return false;
        else
            ch->mana -= mana;
    }

    bool fForbidCasting = false;
    bool fForbidReaction = false;
    bool offensive = spell->getSpellType( ) == SPELL_OFFENSIVE;

    if (IS_SET(flags, FSPELL_VERBOSE))
        spell->utter( ch );

    if (IS_SET(flags, FSPELL_WAIT)) 
        ch->setWait( spell->getBeats(ch) );

    if (IS_SET(flags, FSPELL_CHECK_SAFE) && offensive && target->victim) {
        if (is_safe(ch, target->victim))
            return false;
    }

    if (IS_SET(flags, FSPELL_CHECK_GROUP) && offensive && target->victim) {
        if (is_same_group(ch, target->victim))
            return false;
    }

    if (IS_SET(flags, FSPELL_BANE) && spell->spellbane( ch, target->victim ))
        return false;
   
    if (!IS_SET(flags, FSPELL_NOTRIGGER)) {
        if (target->type == SpellTarget::CHAR && target->victim)
            fForbidCasting = mprog_spell( target->victim, ch, skill, true );
        
        mprog_cast( ch, target, skill, true );
    }

    if (!fForbidCasting)
        spell->run( ch, target, level );

    if (!IS_SET(flags, FSPELL_NOTRIGGER)) {
        if (target->type == SpellTarget::CHAR && target->victim)
            fForbidReaction = mprog_spell( target->victim, ch, skill, false );

        mprog_cast( ch, target, skill, false );
    }

    if (fForbidReaction)
        return true;

    if (IS_SET(flags, FSPELL_ATTACK_CASTER) && offensive && target->victim) {
        attack_caster(ch, target->victim);
    }

    return true;
}

bool spell( int sn, int level, Character *ch, SpellTarget::Pointer target, int flags )
{
    try { 
        return spell_nocatch(sn, level, ch, target, flags);
    } catch (const VictimDeathException &e) {
        return true;
    }
}

bool spell( int sn, int level, Character *ch, Character *victim, int flags )
{
    return spell( sn, level, ch, SpellTarget::Pointer( NEW, victim ), flags );
}
bool spell_nocatch( int sn, int level, Character *ch, Character *victim, int flags )
{
    return spell_nocatch( sn, level, ch, SpellTarget::Pointer( NEW, victim ), flags );
}
bool spell( int sn, int level, Character *ch, Object *obj )
{
    return spell( sn, level, ch, SpellTarget::Pointer( NEW, obj ) );
}
bool spell( int sn, int level, Character *ch, Room *room )
{
    return spell( sn, level, ch, SpellTarget::Pointer( NEW, room ) );
}
bool spell( int sn, int level, Character *ch, char *arg )
{
    return spell( sn, level, ch, SpellTarget::Pointer( NEW, arg ) );
}

/*
 * cast a spell using pills or potions
 */
void spell_by_item( Character *ch, Object *obj )
{
    for (int i = 1; i <= 4; i++) {
        Skill *skill;
        Spell::Pointer spell;
        SpellTarget::Pointer result( NEW );
        int target;

        skill = SkillManager::getThis( )->find( obj->valueByIndex(i) );

        if (!skill)
            continue;

        spell = skill->getSpell( );
        
        if (!spell) {
            LogStream::sendError( ) << "obj [" << obj->pIndexData->vnum << "] has bad spell #" << i << endl;
            continue;
        }
        
        target = spell->getTarget( );

        if (IS_SET(target, TAR_IGNORE|TAR_CREATE_OBJ|TAR_CREATE_MOB)) {
            result->type = SpellTarget::NONE;
            result->arg = DLString::emptyString;
        }
        else if (IS_SET(target, TAR_ROOM|TAR_PEOPLE)) {
            result->type = SpellTarget::ROOM;
            result->room = ch->in_room;
        }
        else if (IS_SET(target, TAR_CHAR_SELF)) {
            result->type = SpellTarget::CHAR;
            result->victim = ch;
        }
        else if (IS_SET(target, TAR_CHAR_ROOM)) {
            result->type = SpellTarget::CHAR;

            if (spell->getSpellType( ) == SPELL_OFFENSIVE && ch->fighting)
                result->victim = ch->fighting;
            else
                result->victim = ch;
        }
        else {
            LogStream::sendError( ) << "obj [" << obj->pIndexData->vnum << "] has bad spell target #" << i << endl;
            continue;
        }
        

        try {
            if (!spell->spellbane( ch, result->victim )) {
                bool fForbidCasting = false;

                if (result->type == SpellTarget::CHAR && result->victim)
                    fForbidCasting = mprog_spell( result->victim, ch, skill, true );
        
                mprog_cast( ch, result, skill, true );

                if (!fForbidCasting)
                    spell->run( ch, result, obj->value0() );

                if (result->type == SpellTarget::CHAR && result->victim)
                    mprog_spell( result->victim, ch, skill, false );

                mprog_cast( ch, result, skill, false );
            }
        } catch (const VictimDeathException &e) {
            break;
        }
    }
}

bool savesDispel( int dis_level, int spell_level, int duration)
{
    int save;

    /* impossible to dispel permanent effects */
    if (duration == -2 ) return 1;
    if (duration == -1 ) spell_level += 5;

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

bool checkDispel( int dis_level, Character *victim, int sn)
{
    for (auto af: victim->affected.clone()) {
        if (af->type != sn) 
            continue;

        if (!savesDispel(dis_level,af->level,af->duration)) {
            AffectHandler::Pointer handler = skillManager->find( sn )->getAffect( );
            
            if (handler) {
                handler->onDispel(SpellTarget::Pointer(NEW, victim), af);
                handler->onRemove(SpellTarget::Pointer(NEW, victim), af);
            }

            affect_strip(victim,sn);            
            return true;
        }
        else
            af->level--;
    }

    return false;
}

bool is_safe_spell(Character *ch, Character *victim, bool area )
{
    if (ch == victim && !area)
        return true;

    if (victim->is_immortal() &&  area)
        return true;

    if (is_same_group(ch,victim) && area)
        return true;

    if (ch == victim && area && ch->in_room->getSectorType() == SECT_INSIDE)
        return true;

    if ((RIDDEN(ch) == victim || MOUNTED(ch) == victim) && area)
        return true;

    return is_safe(ch,victim);
}

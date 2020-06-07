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

#include "behavior_utils.h"
#include "logstream.h"
#include "spell.h"
#include "spelltarget.h"
#include "skillmanager.h"
#include "skill.h"
#include "affecthandler.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"

#include "magic.h"
#include "clanreference.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "handler.h"
#include "interp.h"
#include "vnum.h"
#include "fight_exception.h"
#include "immunity.h"
#include "material.h"
#include "fight.h"
#include "def.h"

CLAN(none);

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( short level, Character *victim, int dam_type, Character *ch, bitstring_t dam_flag )
{
    int save;
    int mlevel = victim->getModifyLevel( );

    save = 40 + (mlevel - level) * 4 -
        (victim->saving_throw * 90) / max( 45, mlevel );

    if (IS_AFFECTED(victim,AFF_BERSERK))
        save += mlevel / 5;
    
    switch(immune_check(victim, dam_type, dam_flag)) {
    case RESIST_IMMUNE:                return true;
    case RESIST_RESISTANT:        save += mlevel / 5; break;
    case RESIST_VULNERABLE:        save -= mlevel / 5; break;
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
        for (Affect *paf = ch->affected; paf; paf = paf->next)
            if (paf->type->getAffect( ))
                paf->type->getAffect( )->saves( ch, victim, save, dam_type, paf );

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

        if (!IS_AWAKE(wch))
            continue;
            
        if (!everywhere && !IS_OUTSIDE(wch))
            continue;
        
        wch->println( msg );
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

    if (!target)
        return false;
    
    if (!( skill = SkillManager::getThis( )->find( sn ) ))
        return false;

    if (!( spell = skill->getSpell( ) ))
        return false;
    
    if (IS_SET(flags, FSPELL_OBSTACLES)) {
        if ((ch->isAffected( gsn_garble ) || ch->isAffected( gsn_deafen )) && chance( 50 ))
            return false;
        
        if (ch->isAffected( gsn_shielding ) && chance( 50 ))
            return false;

        if (target->type == SpellTarget::CHAR
             && target->victim->in_room != ch->in_room
             && !ch->can_see( target->victim ))
            return false;
    }
    
    if (IS_SET(flags, FSPELL_MANA)) {
        int mana = spell->getManaCost( ch ) / 2;

        if (ch->mana < mana)
            return false;
        else
            ch->mana -= mana;
    }

    if (IS_SET(flags, FSPELL_VERBOSE))
        spell->utter( ch );

    if (IS_SET(flags, FSPELL_WAIT)) 
        ch->setWait( spell->getBeats( ) );

    if (IS_SET(flags, FSPELL_BANE) && spell->spellbane( ch, target->victim ))
        return false;
   
    bool fForbidCasting = false;

    if (!IS_SET(flags, FSPELL_NOTRIGGER)) {
        if (target->type == SpellTarget::CHAR && target->victim)
            fForbidCasting = mprog_spell( target->victim, ch, skill, true );
        
        mprog_cast( ch, target, skill, true );
    }

    if (!fForbidCasting)
        spell->run( ch, target, level );

    if (!IS_SET(flags, FSPELL_NOTRIGGER)) {
        if (target->type == SpellTarget::CHAR && target->victim)
            mprog_spell( target->victim, ch, skill, false );

        mprog_cast( ch, target, skill, false );
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
            result->arg = "";
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
    Affect *af;

    for (af = victim->affected; af != 0; af = af->next) {
        if (af->type != sn) 
            continue;

        if (!savesDispel(dis_level,af->level,af->duration)) {
            AffectHandler::Pointer handler = skillManager->find( sn )->getAffect( );
            
            if (handler) {
                handler->remove( victim );
                handler->dispel( victim );
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

    if (ch == victim && area && ch->in_room->sector_type == SECT_INSIDE)
        return true;

    if ((RIDDEN(ch) == victim || MOUNTED(ch) == victim) && area)
        return true;

    return is_safe(ch,victim);
}


bool overcharmed( Character *ch )        
{
    Character *gch;
    int count, max_charm;

    max_charm  = ch->getCurrStat( STAT_INT ) / 4 + ch->getRealLevel( ) / 30;
    max_charm -= 28 - min( 28, ch->getCurrStat( STAT_CHA ) );

    count = 0;

    for (gch = char_list; gch != 0; gch = gch->next) {
        if (IS_CHARMED(gch) && gch->master == ch)
            count++;
    }

    if (count >= max_charm) {
        ch->printf( "Ты уже контролируешь {C%d{x последователей из {c%d{x возможных.\n\r", 
                    count, max_charm );        
        return true;
    }

    return false;
}

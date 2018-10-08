/* $Id: caster.cpp,v 1.1.2.12 2010-09-01 21:20:43 rufina Exp $
 *
 * ruffina, 2007
 */
#include "basicmobilebehavior.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "spell.h"
#include "spelltarget.h"

#include "fight.h"
#include "gsn_plugin.h"
#include "magic.h"
#include "act.h"
#include "interp.h"
#include "act_move.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#ifndef AI_STUB
GSN(summon);
GSN(none);
GSN(stardust);
GSN(dispel_affects);
PROF(warlock);
PROF(necromancer);
PROF(cleric);

static inline bool has_sanctuary( Character *ch )
{
    return IS_AFFECTED(ch, AFF_SANCTUARY)
                    || ch->isAffected(gsn_stardust)
                    || ch->isAffected(gsn_dark_shroud);
}

/*-----------------------------------------------------------------------------
 * common caster brain 
 *----------------------------------------------------------------------------*/
Character * BasicMobileBehavior::findCastVictim( )
{
    Character *victim = NULL, *rch;
    int chHpPercent = HEALTH(ch);

    for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room) {
        int switch_chance = 0;

        if (rch->fighting != ch && ch->fighting != rch)
            continue;
        
        if (ch->fighting == rch && chHpPercent < 25)
            return rch;
        
        if (!victim)
            switch_chance = 100;
        else {
            switch_chance += 60 * (victim->hit > rch->hit);
            switch_chance += 30 * (has_sanctuary( victim ) 
                                   && !has_sanctuary( rch ));
        }

        if (number_percent( ) < switch_chance)
            victim = rch;
    }
    
    if (chance( 60 ))
        return victim;
    else
        return NULL;
}

/*
 * special: fight
 */
bool BasicMobileBehavior::specFightCaster( )
{
    int prof = -1, cnt = 0;
    
    if (ch->wait > 0)
        return false;
    
    if (IS_SET( ch->act, ACT_VAMPIRE )) {
        if (IS_SET( ch->act, ACT_UNDEAD|ACT_NECROMANCER ) && chance( 50 ))
            return specFightNecro( );
        else
            return specFightVampire( );
    }
    
    if (IS_SET( ch->act, ACT_CLERIC )) {
        if (number_range( 0, cnt++ ) == 0) 
            prof = prof_cleric;
    }

    if (IS_SET( ch->act, ACT_MAGE )) {
        if (number_range( 0, cnt++ ) == 0) 
            prof = prof_warlock;
    }
    
    if (IS_SET( ch->act, ACT_UNDEAD|ACT_NECROMANCER )) {
        if (number_range( 0, cnt++ ) == 0) 
            prof = prof_necromancer;
    }
    
    if (prof == prof_cleric)
        return specFightCleric( );

    if (prof == prof_warlock)
        return specFightMage( );

    if (prof == prof_necromancer)
        return specFightNecro( );
    
    return specFight( );
}

/*
 * tracking
 */
bool BasicMobileBehavior::trackCaster( Character *wch )
{
    if (isAfterCharm( ))
        return false;

    if (hasDestiny( ))
        return false;

    if (ch->getRealLevel( ) < 75)
        return false;

    if (!isAdrenalined( ))
        return false;

    if (!lostTrack && chance( 90 )) 
        return false;

    if (!gsn_summon->usable( ch )) 
        return false;

    if (!::spell( gsn_summon, ch->getModifyLevel( ), 
                  ch, wch, FSPELL_VERBOSE | FSPELL_WAIT | FSPELL_OBSTACLES | FSPELL_MANA ))
        return false;

    if (wch->in_room != ch->in_room)
        return false;
    
    return true;
}

/*
 * ranged aggression
 */
bool BasicMobileBehavior::aggressCaster( )
{
    for (int range = 1; range < ch->getModifyLevel( ) / 10; range++) {
        int victDoor, victRange, castSn;
        Character *victim;

        if (!( victim = findRangeVictim( range, victDoor, victRange ) )) 
            continue;

        if (( castSn = casterSnRange( victim, victRange ) ) == -1)  
            continue;
    
        SpellTarget::Pointer target( NEW );
        target->type = SpellTarget::CHAR;            
        target->victim = victim;
        target->castFar = true;
        target->range = victRange;

        act("$c1 пристально смотрит $T.", ch, 0, dirs[victDoor].leave, TO_ROOM);
        return ::spell( castSn, ch->getModifyLevel( ), ch, target, 
                        FSPELL_VERBOSE | FSPELL_BANE | FSPELL_WAIT | FSPELL_OBSTACLES | FSPELL_MANA );
    }

    return false;
}

bool BasicMobileBehavior::canAggressDistanceCaster( Character *victim )
{
    return casterSnRange( victim, 1 ) != -1;
}

int BasicMobileBehavior::casterSnRange( Character *victim, int victRange )
{
    int castSn = -1;

    if (IS_SET(ch->act, ACT_MAGE))
        castSn = SpellChanceTable( mageSnRange, ch, victim ).findRangedSpell( victRange );

    if (castSn == -1 && IS_SET(ch->act, ACT_CLERIC))
        castSn = SpellChanceTable( clericSnRange, ch, victim ).findRangedSpell( victRange );
    
    if (castSn == -1 && IS_SET(ch->act, ACT_UNDEAD|ACT_NECROMANCER))
        castSn = SpellChanceTable( necroSnRange, ch, victim ).findRangedSpell( victRange );

    return castSn;
}

/*-----------------------------------------------------------------------------
 * SpellChanceTable
 *----------------------------------------------------------------------------*/
BasicMobileBehavior::SpellChanceTable::SpellChanceTable( const SpellChance *st, NPCharacter *c, Character *v )
                        : spellTable( st ), ch( c ), victim( v )
{
}

int BasicMobileBehavior::SpellChanceTable::findSpell( )
{
    return findRangedSpell( 0 );
}

int BasicMobileBehavior::SpellChanceTable::findRangedSpell( int victRange )
{
    int i;
    Skill *skill;

    for (i = 0; spellTable[i].chance != -1; i++) {
        if (!( skill = skillManager->find( spellTable[i].gsn ) ))
            continue;
        if (!skill->usable( ch ) || !skill->getSpell( ) || !skill->getSpell( )->isCasted( ))
            continue;
        if (!canCastSpell( spellTable[i].gsn ))
            continue;
        if (skill->getSpell( )->getMaxRange( ch ) < victRange)
            continue;
        if (!chance( spellTable[i].chance ))
            continue;
        
        break;
    }

    return (spellTable[i].gsn == gsn_none ? -1 : spellTable[i].gsn);
}

bool BasicMobileBehavior::SpellChanceTable::castSpell( int noFlags )
{
    int sn = findSpell( );
    int flags = (FSPELL_VERBOSE | FSPELL_BANE | FSPELL_WAIT 
                 | FSPELL_OBSTACLES | FSPELL_MANA) & noFlags;

    if (sn == -1)
        return false;
    
    return ::spell( sn, ch->getModifyLevel( ), ch, victim, flags );
}

bool BasicMobileBehavior::SpellChanceTable::canCastSpell( int sn )
{   
    /*
     * dispel magic
     */
    if (sn == gsn_dispel_affects) {
        int chance = 0;

        if (victim->isAffected( gsn_spellbane ))
            return false;
         
        chance += 50 * (victim->isAffected( gsn_sanctuary ));
        chance += 50 * (victim->isAffected( gsn_stardust ));
        chance += 40 * (victim->isAffected( gsn_dark_shroud ));
        chance += 30 * (IS_GOOD(ch) && victim->isAffected( gsn_protection_good ));
        chance += 30 * (IS_EVIL(ch) && victim->isAffected( gsn_protection_evil ));
        chance += 10 * (victim->isAffected( gsn_benediction ));
        chance += 10 * (victim->isAffected( gsn_haste ));
        chance += 10 * (victim->isAffected( gsn_bless ));
        chance += 10 * (victim->isAffected( gsn_inspire ));
        chance += 10 * (victim->isAffected( gsn_frenzy ));
        chance -= 99 * (victim->isAffected( gsn_corruption ));
        chance -= 75 * (victim->isAffected( gsn_blindness ));
        chance -= 75 * (victim->isAffected( gsn_web ));
        chance -= 30 * (victim->isAffected( gsn_poison ));
        chance -= 30 * (victim->isAffected( gsn_faerie_fire ));

        return number_percent( ) < chance;
    }
    
    /*
     * web
     */
    if (sn == gsn_web) {
        if (victim->isAffected( gsn_web ))
            return false;

        for (int d = 0; d < DIR_SOMEWHERE; d++)
            if (victim->in_room->exit[d])
                if (!IS_SET(victim->in_room->exit[d]->exit_info, EX_NOFLEE))
                    return true;

        return false;
    }
    
    /*
     * attack spells
     */
    if (sn == gsn_ray_of_truth)
        return !IS_EVIL( ch ) && IS_EVIL( victim );
    
    if (sn == gsn_holy_word)
        return (IS_GOOD( ch ) && IS_EVIL( victim ))
                || (IS_EVIL( ch ) && IS_GOOD( victim ));
    
    if (sn == gsn_energy_drain)
        return !IS_GOOD( ch );

    if (sn == gsn_bluefire)
        return IS_NEUTRAL( ch );

    if (sn == gsn_demonfire)
        return IS_EVIL( ch );

    if (sn == gsn_dispel_evil)
        return IS_GOOD( ch ) && IS_EVIL( victim );

    if (sn == gsn_dispel_good)
        return IS_EVIL( ch ) && IS_GOOD( victim );
    
    /*
     * maladictions
     */
    if (sn == gsn_faerie_fire)
        return !IS_AFFECTED( victim, AFF_FAERIE_FIRE );

    if (sn == gsn_blindness)
        return !IS_AFFECTED( victim, AFF_BLIND );

    if (sn == gsn_curse)
        return !IS_AFFECTED( victim, AFF_CURSE );

    if (sn == gsn_slow)
        return !IS_AFFECTED( victim, AFF_SLOW );

    if (sn == gsn_weaken)
        return !victim->isAffected( sn );

    if (sn == gsn_corruption)
        return !IS_AFFECTED( victim, AFF_CORRUPTION );

    
    /*
     * curative 
     */
    if (sn == gsn_cure_poison)
        return victim->isAffected( gsn_poison );

    if (sn == gsn_cure_disease)
        return victim->isAffected( gsn_plague );

    if (sn == gsn_cure_blindness)
        return victim->isAffected( gsn_blindness );

    if (sn == gsn_remove_curse)
        return victim->isAffected( gsn_curse );

    return true;
}


#endif

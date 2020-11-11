/* $Id$
 *
 * ruffina, 2004
 */
#include "loadsave.h"
#include "save.h"

#include "skillreference.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "race.h"

#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(none);
WEARLOC(stuck_in);

/*
 * Apply or remove an affect to an object.
 */
void affect_remove_obj( Object *obj, Affect *paf )
{
    int where, vector;
    if ( obj->affected.empty())
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (obj->carried_by != NULL && obj->wear_loc != wear_none)
        affect_modify( obj->carried_by, paf, false );

    where = paf->where;
    vector = paf->bitvector;

    if (paf->bitvector)
        switch( paf->where)
        {
        case TO_OBJECT:
            REMOVE_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                obj->value4(obj->value4() & ~paf->bitvector);
            break;
        }

    obj->affected.remove(paf);
    ddeallocate( paf );

    if (obj->carried_by != NULL && obj->wear_loc != wear_none)
        affect_check(obj->carried_by,where,vector);
}

void affect_to_obj( Object *obj, Affect *paf )
{
    obj->affected.push_front(paf->clone());

    if (paf->bitvector)
        switch (paf->where)
        {
        case TO_OBJECT:
            SET_BIT(obj->extra_flags,paf->bitvector);
            break;
        case TO_WEAPON:
            if (obj->item_type == ITEM_WEAPON)
                obj->value4(obj->value4() | paf->bitvector);
            break;
        }
}

void affect_enhance( Object *obj, Affect *newAff )
{
    for (auto &paf: obj->affected) {
        if (paf->location != newAff->location)
            continue;
        
        if (paf->duration * newAff->duration <= 0)
            continue;

        paf->type      = newAff->type;
        paf->modifier += newAff->modifier;
        paf->level     = max( paf->level,    newAff->level );
        paf->duration  = max( paf->duration, newAff->duration );

        return;
    }

    affect_to_obj( obj, newAff );
}

void affect_enchant( Object *obj )
{
    if (!obj->enchanted) {
        Affect af;

        obj->enchanted = true;

        for (auto &paf: obj->pIndexData->affected) {
            af = *paf;
            
            af.type = paf->type;
            affect_to_obj( obj, &af );
        }
    }
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( Character *ch, Affect *paf, bool fAdd )
{
    int mod,i;

    mod = paf->modifier;

    if ( fAdd )
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
             ch->affected_by.setBit(paf->bitvector);
            break;
        case TO_IMMUNE:
            ch->imm_flags.setBit(paf->bitvector);
            break;
        case TO_RESIST:
            ch->res_flags.setBit(paf->bitvector);
            break;
        case TO_ACT_FLAG:
            ch->act.setBit(paf->bitvector);
            break;
        case TO_VULN:
            ch->vuln_flags.setBit(paf->bitvector);
            break;
        case TO_DETECTS:
            ch->detection.setBit(paf->bitvector);
            break;
        case TO_LOCATIONS:
            if (!ch->is_npc( ))
                ch->getPC( )->wearloc.remove( paf->global );
            break;
        case TO_RACE:
            break;
        }
    }
    else
    {
        switch (paf->where)
        {
        case TO_AFFECTS:
            ch->affected_by.removeBit(paf->bitvector);
            break;
        case TO_IMMUNE:
            ch->imm_flags.removeBit(paf->bitvector);
            break;
        case TO_RESIST:
            ch->res_flags.removeBit(paf->bitvector);
            break;
        case TO_ACT_FLAG:
            ch->act.removeBit(paf->bitvector);
            break;
        case TO_VULN:
            ch->vuln_flags.removeBit(paf->bitvector);
            break;
        case TO_DETECTS:
            ch->detection.removeBit(paf->bitvector);
            break;
        case TO_LOCATIONS:
            if (!ch->is_npc( ))
                ch->getPC( )->wearloc.set( paf->global );
            break;
        case TO_RACE:
            break;
        }
        mod = 0 - mod;
    }

    switch ( paf->location )
    {
    default:
        bug( "Affect_modify: unknown location %d.", paf->location );
        return;

    case APPLY_STR:           ch->mod_stat[STAT_STR]        += mod;        break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]        += mod;        break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]        += mod;        break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]        += mod;        break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]        += mod;        break;
    case APPLY_CHA:              ch->mod_stat[STAT_CHA]        += mod; break;
    case APPLY_CLASS:                                                break;
    case APPLY_AGE:
        if (!ch->is_npc( ))
            ch->getPC( )->age.modifyYears( mod );
        break;
    case APPLY_HEIGHT:                                                break;
    case APPLY_WEIGHT:                                                break;
    case APPLY_MANA:          ch->max_mana                += mod;        break;
    case APPLY_HIT:           ch->max_hit                += mod;        break;
    case APPLY_MOVE:          ch->max_move                += mod;        break;
    case APPLY_GOLD:                                                break;
    case APPLY_EXP:                                                break;
    case APPLY_AC:
        for (i = 0; i < 4; i ++)
            ch->armor[i] += mod;
        break;
    case APPLY_HITROLL:       ch->hitroll                += mod;        break;
    case APPLY_DAMROLL:       ch->damroll                += mod;        break;
    case APPLY_SIZE:                ch->size                += mod; break;
    case APPLY_SAVES:   ch->saving_throw                += mod;        break;
    case APPLY_SAVING_ROD:    ch->saving_throw                += mod;        break;
    case APPLY_SAVING_PETRI:  ch->saving_throw                += mod;        break;
    case APPLY_SAVING_BREATH: ch->saving_throw                += mod;        break;
    case APPLY_SAVING_SPELL:  ch->saving_throw                += mod;        break;
    case APPLY_MANA_GAIN:     ch->mana_gain             += mod; break;
    case APPLY_HEAL_GAIN:     ch->heal_gain             += mod; break;

    case APPLY_NONE:
    case APPLY_LEARNED: 
        if (!ch->is_npc()) {
            switch (paf->where) {
                case TO_SKILLS:
                    ch->getPC()->mod_skills.applyBitvector(paf->global, mod);
                    break;
                case TO_SKILL_GROUPS:
                    ch->getPC()->mod_skill_groups.applyBitvector(paf->global, mod);
                    break;
            }
        }
        break;

    case APPLY_LEVEL:
        if (!ch->is_npc()) {
            switch (paf->where) {
                case TO_SKILLS:
                    ch->getPC()->mod_level_groups.applyBitvector(paf->global, mod);
                    break;
                case TO_SKILL_GROUPS:
                    ch->getPC()->mod_level_groups.applyBitvector(paf->global, mod);
                    break;
                default:
                    ch->getPC()->mod_level_all += mod;
                    break;
            }
        }
        break;
    }
}


/* fix object affects when removing one */
void affect_check(Character *ch,int where,int vector)
{
        Object *obj;

        if (where == TO_OBJECT || where == TO_WEAPON || vector == 0)
                return;

        for (auto &paf: ch->affected)
                if (paf->where == where && ( ( paf->bitvector & vector ) > 0 ) )
                {
                        switch (where)
                        {
                                case TO_AFFECTS:
                                        ch->affected_by.setBit(paf->bitvector);
                                        break;
                                case TO_IMMUNE:
                                        ch->imm_flags.setBit(paf->bitvector);
                                        break;
                                case TO_RESIST:
                                        ch->res_flags.setBit(paf->bitvector);
                                        break;
                                case TO_ACT_FLAG:
                                        ch->act.setBit(paf->bitvector);
                                        break;
                                case TO_VULN:
                                        ch->vuln_flags.setBit(paf->bitvector);
                                        break;
                                case TO_DETECTS:
                                        ch->detection.setBit(paf->bitvector);
                                        break;
                                case TO_LOCATIONS:
                                        if (!ch->is_npc( ))
                                            ch->getPC( )->wearloc.remove( paf->global );
                                        break;
                                case TO_RACE:
                                        break;
                        }
                }

        for (obj = ch->carrying; obj != 0; obj = obj->next_content)
        {
                if (obj->wear_loc == wear_none || obj->wear_loc == wear_stuck_in)
                        continue;

                for (auto &paf: obj->affected)
                        if ( paf->where == where && ( ( paf->bitvector & vector ) > 0 ) )
                        {
                                switch (where)
                                {
                                        case TO_AFFECTS:
                                                ch->affected_by.setBit(paf->bitvector);
                                                break;
                                        case TO_IMMUNE:
                                                ch->imm_flags.setBit(paf->bitvector);
                                                break;
                                        case TO_ACT_FLAG:
                                                ch->act.setBit(paf->bitvector);
                                                break;
                                        case TO_RESIST:
                                                ch->res_flags.setBit(paf->bitvector);
                                                break;
                                        case TO_VULN:
                                                ch->vuln_flags.setBit(paf->bitvector);
                                                break;
                                        case TO_DETECTS:
                                                ch->detection.setBit(paf->bitvector);
                                                break;
                                        case TO_LOCATIONS:
                                            if (!ch->is_npc( ))
                                                ch->getPC( )->wearloc.remove( paf->global );
                                            break;
                                        case TO_RACE:
                                                break;
                                }
                        }

                if (obj->enchanted)
                        continue;

                for (auto &paf: obj->pIndexData->affected)
                        if (paf->where == where && ( ( paf->bitvector & vector ) > 0 ))
                        {
                                switch (where)
                                {
                                        case TO_AFFECTS:
                                                ch->affected_by.setBit(paf->bitvector);
                                                break;
                                        case TO_IMMUNE:
                                                ch->imm_flags.setBit(paf->bitvector);
                                                break;
                                        case TO_ACT_FLAG:
                                                ch->act.setBit(paf->bitvector);
                                                break;
                                        case TO_RESIST:
                                                ch->res_flags.setBit(paf->bitvector);
                                                break;
                                        case TO_VULN:
                                                ch->vuln_flags.setBit(paf->bitvector);
                                                break;
                                        case TO_DETECTS:
                                                ch->detection.setBit(paf->bitvector);
                                                break;
                                        case TO_LOCATIONS:
                                            if (!ch->is_npc( ))
                                                ch->getPC( )->wearloc.remove( paf->global );
                                            break;
                                        case TO_RACE:
                                                break;

                                }
                        }
        }
    
    RaceReference &race = ch->getRace( );
    
    switch (where) {
    case TO_AFFECTS:
            if (race->getAff( ).isSet( vector ))
                ch->affected_by.setBit(race->getAff( ));
            break;
    case TO_IMMUNE:
            if (race->getImm( ).isSet( vector ))
                ch->imm_flags.setBit(race->getImm( ));
            break;
    case TO_RESIST:
            if (race->getRes( ).isSet( vector ))
                ch->res_flags.setBit(race->getRes( ));
            break;
    case TO_VULN:
            if (race->getVuln( ).isSet( vector ))
                ch->vuln_flags.setBit(race->getVuln( ));
            break;
    case TO_DETECTS:
            if (race->getDet( ).isSet( vector ))
                ch->detection.setBit(race->getDet( ));
            break;
    }

    if (where == TO_AFFECTS && IS_SET(vector, AFF_FLYING))
        if (!ch->affected_by.isSet( AFF_FLYING ))
            ch->posFlags.removeBit( POS_FLY_DOWN );
}

/*
 * Give an affect to a char.
 */
void affect_to_char( Character *ch, Affect *paf )
{
        bool need_saving = false;

        ch->affected.push_front(paf->clone());

        need_saving = ch->is_npc( )
                && paf->where == TO_AFFECTS
                && IS_SET( paf->bitvector, AFF_CHARM )
                && ch->in_room;

        affect_modify( ch, paf, true );

        if ( need_saving )
                save_mobs( ch->in_room );

        return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( Character *ch, Affect *paf )
{
        int where;
        int vector;
        bool need_saving = false;

        if ( ch->affected.empty())
        {
                bug( "Affect_remove: no affect.", 0 );
                return;
        }

        affect_modify( ch, paf, false );

        where = paf->where;
        vector = paf->bitvector;

        need_saving = ch->is_npc( )
                && where == TO_AFFECTS
                && IS_SET( vector, AFF_CHARM )
                && ch->in_room;

        ch->affected.remove(paf);
        ddeallocate( paf );

        affect_check(ch,where,vector);

        if ( need_saving )
                save_mobs( ch->in_room );

        return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip( Character *ch, int sn )
{
    for (auto &paf: ch->affected.findAll(sn))
        affect_remove( ch, paf );
}

void affect_strip( Object *obj, int sn )
{
    for (auto &paf: obj->affected.findAll(sn))
        affect_remove_obj( obj, paf );
}

/*
 * Strip all affects which affect given bitvector
 */
void affect_bit_strip(Character *ch, int where, int bits)
{
    for (auto &paf: ch->affected.findAllWithBits(where, bits))
        affect_remove(ch, paf);
}

/*
 * Add or enhance an affect.
 */
void affect_join( Character *ch, Affect *paf )
{
    for (auto &paf_old: ch->affected)
    {
        if ( paf_old->type == paf->type )
        {
            if (!paf_old->global.empty( )) 
                if (!paf->global.isSet( paf_old->global ))
                    continue;

            paf->level += paf_old->level;
            paf->level /= 2;
            paf->duration += paf_old->duration;
            paf->modifier += paf_old->modifier;

            affect_remove( ch, paf_old );
            break;
        }
    }

    affect_to_char( ch, paf );
}



void postaffect_to_char( Character *ch, int sn, int duration )
{
    Affect af;

    af.where                = TO_AFFECTS;
    af.type             = sn;
    af.level            = ch->getModifyLevel( );
    af.duration         = duration;
    af.bitvector        = 0;
    af.modifier         = 0;
    af.location         = APPLY_NONE;

    affect_to_char(ch, &af);
}  


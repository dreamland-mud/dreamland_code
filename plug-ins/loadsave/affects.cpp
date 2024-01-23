/* $Id$
 *
 * ruffina, 2004
 */
#include "loadsave.h"
#include "save.h"

#include "skillreference.h"
#include "skillgroup.h"
#include "liquid.h"
#include "wearlocation.h"
#include "affect.h"
#include "affecthandler.h"
#include "affectmanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "race.h"

#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(none);
WEARLOC(stuck_in);

Affect * Affect::clone() const
{
    Affect *paf;

    paf = AffectManager::getThis()->getAffect();
    copyTo(*paf);

    return paf;
}

void Affect::copyTo(Affect &target) const
{
    target.type     = type; 
    target.level    = level;
    target.duration = duration;
    target.modifier = modifier;
    
    if (location.getTable() == 0)
        target.location.setTable(&apply_flags);
    else
        target.location.setTable(location.getTable());

    target.location.setValue(location.getValue());
    target.bitvector.setBit(bitvector.getValue());
    target.bitvector.setTable(bitvector.getTable());
    target.global.setRegistry(global.getRegistry());
    target.global.set(global);
    target.sources = sources;
}

/*
 * Apply or remove an affect to an object.
 */
void affect_remove_obj( Object *obj, Affect *paf, bool verbose )
{
    if (paf->isExtracted())
        return;

    if ( obj->affected.empty())
    {
        bug( "Affect_remove_object: no affect.", 0 );
        return;
    }

    if (verbose) {
        if (paf->type->getAffect())
            paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, obj), paf);
    }

    if (obj->carried_by != NULL && obj->wear_loc != wear_none)
        affect_modify( obj->carried_by, paf, false );

    const FlagTable *table = paf->bitvector.getTable();
    int bits = paf->bitvector.getValue();

    if (table == &extra_flags) {
        REMOVE_BIT(obj->extra_flags, bits);
    } else if (table == &weapon_type2) {
        if (obj->item_type == ITEM_WEAPON)
            obj->value4(obj->value4() & ~bits);
    }

    obj->affected.remove(paf);

    if (obj->carried_by != NULL && obj->wear_loc != wear_none)
        affect_check(obj->carried_by, paf);

    AffectManager::getThis()->extract(paf);
}

void affect_to_obj( Object *obj, const Affect *paf )
{
    // Remove old affects given by this object to the carrier.
    if (obj->carried_by) 
        obj->wear_loc->affectsOnUnequip(obj->carried_by, obj);

    obj->affected.push_front(paf->clone());

    if (paf->bitvector.getTable() == &extra_flags) {
        SET_BIT(obj->extra_flags,paf->bitvector);
    } else if (paf->bitvector.getTable() == &weapon_type2) {
        if (obj->item_type == ITEM_WEAPON)
            obj->value4(obj->value4() | paf->bitvector);
    }

    // Reset updated affect back onto the carrier.
    if (obj->carried_by)
        obj->wear_loc->affectsOnEquip(obj->carried_by, obj);
}

void affect_enhance( Object *obj, const Affect *newAff )
{
    for (auto &paf: obj->affected) {
        if (paf->location != newAff->location)
            continue;
        
        if (paf->duration * newAff->duration <= 0)
            continue;

        if(paf->type.getName() != newAff->type.getName())
            continue;

        // Remove old affects given by this object to the carrier.
        if (obj->carried_by)
            obj->wear_loc->affectsOnUnequip(obj->carried_by, obj);

        paf->modifier += newAff->modifier;
        paf->level     = max( paf->level,    newAff->level );
        paf->duration  = max( paf->duration, newAff->duration );

        // Reset updated affect back onto the carrier.
        if (obj->carried_by)
            obj->wear_loc->affectsOnEquip(obj->carried_by, obj);

        return;
    }

    affect_to_obj( obj, newAff );
}

static Flags zeroFlags;

static Flags & char_flag_by_table(Character *ch, const FlagTable *table)
{
    if (table == &affect_flags)
        return ch->affected_by;
    else if (table == &imm_flags)
        return ch->imm_flags;
    else if (table == &res_flags)
        return ch->res_flags;
    else if (table == &plr_flags || table == &act_flags)
        return ch->act;
    else if (table == &vuln_flags)
        return ch->vuln_flags;
    else if (table == &detect_flags)
        return ch->detection;
    else if (table == &form_flags)
        return ch->form;
    else if (table == &part_flags)
        return ch->parts;    

    return zeroFlags;
}

static const Flags & race_flag_by_table(const Race *race, const FlagTable *table)
{
    if (table == &affect_flags)
        return race->getAff();
    else if (table == &imm_flags)
        return race->getImm();
    else if (table == &res_flags)
        return race->getRes();
    else if (table == &plr_flags || table == &act_flags)
        return race->getAct();
    else if (table == &vuln_flags)
        return race->getVuln();
    else if (table == &detect_flags)
        return race->getDet();
    else if (table == &form_flags)
        return race->getForm();
    else if (table == &part_flags)
        return race->getParts();    

    return Flags::emptyFlags;
}

/*
 * Apply or remove an affect to a character.
 */
void affect_modify( Character *ch, Affect *paf, bool fAdd )
{
    int mod, i;
    const FlagTable *table = paf->bitvector.getTable();

    if (table) {
        char_flag_by_table(ch, table).changeBit(paf->bitvector, fAdd);
    }

    if (paf->global.getRegistry() == wearlocationManager) {
        ch->wearloc.change(paf->global, !fAdd);
    }

    mod = (fAdd ? 1 : -1) * paf->modifier;

    switch (paf->location)
    {
    default:
        bug( "Affect_modify: unknown location %d.", paf->location.getValue() );
        return;

    case APPLY_STR:           ch->mod_stat[STAT_STR]        += mod;        break;
    case APPLY_DEX:           ch->mod_stat[STAT_DEX]        += mod;        break;
    case APPLY_INT:           ch->mod_stat[STAT_INT]        += mod;        break;
    case APPLY_WIS:           ch->mod_stat[STAT_WIS]        += mod;        break;
    case APPLY_CON:           ch->mod_stat[STAT_CON]        += mod;        break;
    case APPLY_CHA:           ch->mod_stat[STAT_CHA]        += mod; break;
    case APPLY_AGE:
        if (!ch->is_npc( ))
            ch->getPC( )->age.modifyYears( mod );
        break;
    case APPLY_MANA:          ch->max_mana                += mod;        break;
    case APPLY_HIT:           ch->max_hit                += mod;        break;
    case APPLY_MOVE:          ch->max_move                += mod;        break;
    case APPLY_GOLD:                                                break;
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
    case APPLY_BEATS:         ch->mod_beats += mod; break;

    case APPLY_NONE:
    case APPLY_LEARNED: 
        if (!ch->is_npc()) {
            if (paf->global.getRegistry() == skillManager)
                ch->getPC()->mod_skills.applyBitvector(paf->global, mod);
            else if (paf->global.getRegistry() == skillGroupManager)
                ch->getPC()->mod_skill_groups.applyBitvector(paf->global, mod);
            else if (paf->location == APPLY_LEARNED)
                ch->getPC()->mod_skill_all += mod;
        }
        break;

    case APPLY_LEVEL:
        if (!ch->is_npc()) {
            if (paf->global.getRegistry() == skillManager)
                ch->getPC()->mod_level_skills.applyBitvector(paf->global, mod);
            else if (paf->global.getRegistry() == skillGroupManager)
                ch->getPC()->mod_level_groups.applyBitvector(paf->global, mod);
            else
                ch->getPC()->mod_level_all += mod;
        }
        break;

    case APPLY_SPELL_LEVEL:
        if (!ch->is_npc()) {
            ch->getPC()->mod_level_spell += mod;
        }
        break;
    case APPLY_BITVECTOR:
        /* no action */
        break;
    }
}


static void affectlist_reapply(AffectList &afflist, Character *ch, Affect *affect)
{
    const FlagTable *table = affect->bitvector.getTable();
    const GlobalRegistryBase *registry = affect->global.getRegistry();
    Flags &charFlag = char_flag_by_table(ch, table);
    int bits = affect->bitvector.getValue();

    for (auto &paf : afflist)
        if (paf->bitvector.getTable() == table && paf->bitvector.isSet(bits))
            charFlag.setBit(paf->bitvector);
        else if (paf->global.getRegistry() == registry) {
            if (registry == wearlocationManager)
                ch->wearloc.remove(paf->global);
        }
}

/* fix object affects when removing one */
void affect_check(Character *ch, Affect *affect)
{
    const FlagTable *table = affect->bitvector.getTable();
    const GlobalRegistryBase *registry = affect->global.getRegistry();

    if (!table && registry != wearlocationManager)
        return;

    affectlist_reapply(ch->affected, ch, affect);

    for (Object *obj = ch->carrying; obj != 0; obj = obj->next_content) {
        if (!obj->wear_loc->givesAffects())
            continue;

        affectlist_reapply(obj->affected, ch, affect);

        affectlist_reapply(obj->pIndexData->affected, ch, affect);
    }

    if (table) {    
        const Flags &raceFlag = race_flag_by_table(ch->getRace().getElement(), table);
        Flags &charFlag = char_flag_by_table(ch, table);
        charFlag.setBit(raceFlag.getValue());

        if (table == &affect_flags && affect->bitvector.isSet(AFF_FLYING))
            if (!ch->affected_by.isSet(AFF_FLYING))
                ch->posFlags.removeBit(POS_FLY_DOWN);
    }
}

/*
 * Give an affect to a char.
 */
void affect_to_char( Character *ch, Affect *paf )
{
        bool need_saving = false;

        ch->affected.push_front(paf->clone());

        need_saving = ch->is_npc( )
                && paf->bitvector.getTable() == &affect_flags
                && paf->bitvector.isSet(AFF_CHARM)
                && ch->in_room;

        affect_modify( ch, paf, true );

        if ( need_saving )
                save_mobs( ch->in_room );

        return;
}

/*
 * Remove an affect from a char.
 */
void affect_remove( Character *ch, Affect *paf, bool verbose )
{
        if (paf->isExtracted())
            return;
            
        bool need_saving = false;

        if ( ch->affected.empty())
        {
                bug( "Affect_remove: no affect.", 0 );
                return;
        }

        if (verbose) {
            if (paf->type->getAffect())
                paf->type->getAffect()->onRemove(SpellTarget::Pointer(NEW, ch), paf);
        }

        affect_modify( ch, paf, false );

        need_saving = ch->is_npc( )
                && paf->bitvector.getTable() == &affect_flags
                && paf->bitvector.isSet(AFF_CHARM)
                && ch->in_room;

        ch->affected.remove(paf);

        affect_check(ch, paf);

        if ( need_saving )
                save_mobs( ch->in_room );

        AffectManager::getThis()->extract(paf);
}

/*
 * Strip all affects of a given sn. Ensure onRemove trigger is only called
 * once for each affect type.
 */
void affect_strip( Character *ch, int sn, bool verbose )
{
    auto affectsWithType = ch->affected.findAll(sn);

    if (verbose && !affectsWithType.empty()) {
        Affect *firstAffect = affectsWithType.front();
        AffectHandler::Pointer firstHandler = firstAffect->type->getAffect();

        if (firstHandler)
            firstHandler->onRemove(SpellTarget::Pointer(NEW, ch), firstAffect);
    }

    for (auto &paf: affectsWithType)
        affect_remove(ch, paf);
}

void affect_strip( Object *obj, int sn, bool verbose )
{
    for (auto &paf: obj->affected.findAll(sn))
        affect_remove_obj( obj, paf, verbose );
}

/*
 * Strip all affects which affect given bitvector. Some spells add 2-3 affects,
 * only one of them giving a certain bivector (e.g. hitroll+berserk, damroll);
 * in this case need to take care to remove all of them.
 */
void affect_bit_strip(Character *ch, const FlagTable *table, int bits, bool verbose)
{
    list<int> typesToRemove;
    for (auto &paf: ch->affected.findAllWithBits(table, bits))
        typesToRemove.push_back(paf->type);

    for (auto &type: typesToRemove)
        affect_strip(ch, type, verbose);
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
                if (!paf->global.isSetAll( paf_old->global ))
                    continue;

            paf->level = max(paf_old->level, paf->level);
            paf->duration = max(paf_old->duration, paf->duration);
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

    af.type             = sn;
    af.level            = ch->getModifyLevel( );
    af.duration         = duration;
    
    affect_to_char(ch, &af);
}  

/** Compat method for reading affects from old .player profiles. */
const FlagTable * affect_where_to_table(int where)
{
    switch (where) {
    case TO_OBJECT: return &extra_flags;
    case TO_WEAPON: return &weapon_type2;
    case TO_AFFECTS: return &affect_flags;       
    case TO_IMMUNE: return &imm_flags;
    case TO_RESIST: return &res_flags;
    case TO_VULN: return &vuln_flags;
    case TO_ACT_FLAG: return &plr_flags;
    case TO_DETECTS: return &detect_flags;
    case TO_FORM: return &form_flags;
    case TO_ROOM_AFFECTS: return &raffect_flags;
    case TO_ROOM_FLAGS: return &room_flags;
    case TO_PART: return &part_flags;
    default: return 0;
    }
}

/** Compat method for writing affects to the old .player profiles. */
int affect_table_to_where(const FlagTable *table, const GlobalRegistryBase *registry)
{
    if (registry == wearlocationManager) return TO_LOCATIONS;
    if (registry == liquidManager) return TO_LIQUIDS;
    if (registry == skillManager) return TO_SKILLS;
    if (registry == skillGroupManager) return TO_SKILL_GROUPS;

    if (table == &extra_flags) return TO_OBJECT;
    if (table == &weapon_type2) return TO_WEAPON;
    if (table == &affect_flags) return TO_AFFECTS;       
    if (table == &imm_flags) return TO_IMMUNE;
    if (table == &res_flags) return TO_RESIST;
    if (table == &vuln_flags) return TO_VULN;
    if (table == &plr_flags) return TO_ACT_FLAG;
    if (table == &detect_flags) return TO_DETECTS;
    if (table == &form_flags) return TO_FORM;
    if (table == &raffect_flags) return TO_ROOM_AFFECTS;
    if (table == &room_flags) return TO_ROOM_FLAGS;
    if (table == &part_flags) return TO_PART;

    return 0;
}

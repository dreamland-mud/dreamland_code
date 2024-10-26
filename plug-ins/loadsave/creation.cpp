/* $Id$
 *
 * ruffina, 2004
 */
#include "loadsave.h"
#include "logstream.h"
#include "save.h"

#include "fenia/register-impl.h"
#include "fenia_utils.h"
#include "feniamanager.h"
#include "wrapperbase.h"

#include "spelltarget.h"
#include "affecthandler.h"
#include "affect.h"
#include "mobilebehaviormanager.h"
#include "npcharacter.h"
#include "npcharactermanager.h"
#include "object.h"
#include "objectbehaviormanager.h"
#include "objectmanager.h"
#include "pcharacter.h"
#include "race.h"
#include "skillreference.h"

#include "json_utils_ext.h"
#include "act.h"
#include "def.h"
#include "itemevents.h"
#include "merc.h"

#include "vnum.h"

GSN(sanctuary);
GSN(haste);
GSN(protection_evil);
GSN(protection_good);
GSN(stardust);
GSN(dark_shroud);
GSN(corruption);
WEARLOC(none);
WEARLOC(stuck_in);
RELIG(chronos);
PROF(mobile);

typedef void (NPCharacter::*SetterMethod)(const DLString &, lang_t);
/** Override proto descriptions if there's some formatting present. */
static void override_description(NPCharacter *mob, const XMLMultiString &cProtoField, SetterMethod method, lang_t lang)
{
    DLString protoField = cProtoField.get(lang);

    if (protoField.find("%1") == DLString::npos)
        return;

    DLString result = fmt(0, protoField.c_str(), mob);
    if (result != protoField)
        (mob->*method)(result, lang);
}

// Refresh race (and proto) affects on a character. Called when a character is created or periodically from char_update().
void afprog_refresh(Character *ch, bool verbose)
{
    SpellTarget::Pointer spellTarget(NEW, ch);

    for (int sn = 0; sn < skillManager->size(); sn++) {
        bool hasAffect = ch->getRace()->getAffects().isSet(sn);

        if (ch->is_npc()) {
            hasAffect |= ch->getNPC()->pIndexData->affects.isSet(sn);
        }

        if (hasAffect) {
            Skill *skill = skillManager->find(sn);
            AffectHandler::Pointer ah = skill->getAffect();
            if (ah)
                ah->onRefresh(spellTarget, verbose);
            else
                warn("AFFECT %s not found for character %s", skill->getName().c_str(), ch->getNameC());
        }
    }
}

/*
 * Create an instance of a mobile.
 */
NPCharacter *create_mobile(MOB_INDEX_DATA *pMobIndex)
{
    return create_mobile_org(pMobIndex, 0);
}
NPCharacter *create_mobile_nocount(MOB_INDEX_DATA *pMobIndex)
{
    return create_mobile_org(pMobIndex, FCREATE_NOCOUNT);
}
NPCharacter *create_mobile_org(MOB_INDEX_DATA *pMobIndex, int flags)
{
    NPCharacter *mob;
    int i;
    Affect af;
    Race *race;

    if (pMobIndex == 0) {
        bug("Create_mobile: 0 pMobIndex.", 0);
        exit(1);
    }

    mob = NPCharacterManager::getNPCharacter();

    mob->pIndexData = pMobIndex;
    race = raceManager->find(pMobIndex->race);

    mob->spec_fun = pMobIndex->spec_fun;

    if (pMobIndex->wealth == 0) {
        mob->silver = 0;
        mob->gold = 0;
    } else {
        long wealth;

        wealth = number_range(pMobIndex->wealth / 2, 3 * pMobIndex->wealth / 2) / 8;
        mob->gold = number_range(wealth / 2000, wealth / 1000);
        mob->silver = wealth / 10 - (mob->gold * 100);
    }

    /* read from prototype */
    mob->group = pMobIndex->group;
    mob->act = pMobIndex->act | ACT_IS_NPC;
    // FIXME: explicitly apply race affect/detect/resist bits to the mob instance, ignoring 'del' attribute values saved in area XML format.
    // To fix it properly, all mob indexes should have a pair of overrides for every bit field, "bits to delete" and "bits to add".
    mob->affected_by = pMobIndex->affected_by | race->getAff();
    mob->detection = pMobIndex->detection | race->getDet();
    mob->alignment = pMobIndex->alignment;
    mob->setLevel(pMobIndex->level);
    //                mob->hitroll                = (mob->getRealLevel( ) / 2) + pMobIndex->hitroll;
    mob->hitroll = mob->getRealLevel() + pMobIndex->hitroll;
    mob->damroll = pMobIndex->damage[DICE_BONUS];
    mob->max_hit = dice(pMobIndex->hit[DICE_NUMBER], pMobIndex->hit[DICE_TYPE]) + pMobIndex->hit[DICE_BONUS];
    mob->hit = mob->max_hit;
    mob->max_mana = dice(pMobIndex->mana[DICE_NUMBER], pMobIndex->mana[DICE_TYPE]) + pMobIndex->mana[DICE_BONUS];
    mob->mana = mob->max_mana;
    mob->max_move = 200 + 16 * mob->getRealLevel(); // 2 times more than best-trained pc
    mob->move = mob->max_move;
    mob->damage[DICE_NUMBER] = pMobIndex->damage[DICE_NUMBER];
    mob->damage[DICE_TYPE] = pMobIndex->damage[DICE_TYPE];
    mob->dam_type = pMobIndex->dam_type;

    if (mob->dam_type == 0)
        switch (number_range(1, 3)) {
        case (1):
            mob->dam_type = 3;
            break; /* slash */
        case (2):
            mob->dam_type = 7;
            break; /* pound */
        case (3):
            mob->dam_type = 11;
            break; /* pierce */
        }

    for (i = 0; i < 4; i++)
        mob->armor[i] = pMobIndex->ac[i];

    mob->off_flags = pMobIndex->off_flags;
    mob->imm_flags = pMobIndex->imm_flags | race->getImm();
    mob->res_flags = pMobIndex->res_flags | race->getRes();
    mob->vuln_flags = pMobIndex->vuln_flags | race->getVuln();
    mob->wearloc.set(race->getWearloc());
    mob->start_pos = pMobIndex->start_pos;
    mob->default_pos = pMobIndex->default_pos;
    mob->setSex(pMobIndex->sex);
    if (mob->getSex() == SEX_EITHER) /* random sex */
        mob->setSex(number_range(1, 2));

    mob->setRace(race->getName());
    mob->form = pMobIndex->form;
    mob->parts = pMobIndex->parts;
    mob->size = pMobIndex->getSize();
    mob->material = str_dup(pMobIndex->material);
    mob->extracted = false;

    // Override descriptions with formatting symbols that depend on NPC sex.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        override_description(mob, pMobIndex->keyword, &NPCharacter::setKeyword, lang);
        override_description(mob, pMobIndex->short_descr, &NPCharacter::setShortDescr, lang);
        override_description(mob, pMobIndex->long_descr, &NPCharacter::setLongDescr, lang);
        override_description(mob, pMobIndex->description, &NPCharacter::setDescription, lang);
    }

    mob->updateCachedNouns();

    // Configure perm stats, they can be further adjusted in a global onInit.
    // Race and class modifications are applied on-the-fly inside NPCharacter::getCurrStat
    for (i = 0; i < stat_table.size; i++)
        mob->perm_stat[i] = BASE_STAT;

    /* OBSOLETE: let's get some spell action */
    if (!IS_SET(flags, FCREATE_NOAFFECTS))
        create_mob_affects(mob);

    mob->position = mob->start_pos;

    mob->setReligion(god_chronos);
    mob->setProfession(prof_mobile);
    mob->setClan(pMobIndex->clan);
    
    /* link the mob to the world list */
    char_to_list(mob, &char_list);

    if (!IS_SET(flags, FCREATE_NOCOUNT))
        pMobIndex->count++;

    /* assign behavior */
    if (pMobIndex->behavior)
        MobileBehaviorManager::assign(mob);
    else
        MobileBehaviorManager::assignBasic(mob);

    // Let each race or proto affect (silently) do its magic.
    if (!IS_SET(flags, FCREATE_NOAFFECTS))
        afprog_refresh(mob, false);

    /* Fenia initialization: call global onInit followed by specific 'init' defined for this mob index data. */
    if (!IS_SET(flags, FCREATE_NOCOUNT)) {

        gprog("onInit", "C", mob);

        WrapperBase *w = get_wrapper(pMobIndex->wrapper);
        if (w) {
            static Scripting::IdRef initId("init");
            w->call(initId, "C", mob);
        }
    }

    return mob;
}

/** Transform affect bits from the prototype into real affects. Not called for mobs read from disk. */
void create_mob_affects(NPCharacter *mob)
{
    if (IS_AFFECTED(mob, AFF_SANCTUARY)) {
        Affect af;
        if (IS_EVIL(mob)) {
            affect_strip(mob, gsn_sanctuary);
            REMOVE_BIT(mob->affected_by, AFF_SANCTUARY);
            af.type = gsn_dark_shroud;
        } else {
            af.type = gsn_sanctuary;
            af.bitvector.setValue(AFF_SANCTUARY);
            af.bitvector.setTable(&affect_flags);
        }
        af.level = mob->getRealLevel();
        af.duration = -1;
        affect_to_char(mob, &af);
    }

    if (IS_AFFECTED(mob, AFF_HASTE)) {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type = gsn_haste;
        af.level = mob->getRealLevel();
        af.duration = -1;
        af.location = APPLY_DEX;
        af.modifier = 1 + (mob->getRealLevel() >= 18) + (mob->getRealLevel() >= 25) +
                      (mob->getRealLevel() >= 32);
        af.bitvector.setValue(AFF_HASTE);
        affect_to_char(mob, &af);
    }

    if (IS_AFFECTED(mob, AFF_PROTECT_EVIL)) {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type = gsn_protection_evil;
        af.level = mob->getRealLevel();
        af.duration = -1;
        af.location = APPLY_SAVES;
        af.modifier = -1;
        af.bitvector.setValue(AFF_PROTECT_EVIL);
        affect_to_char(mob, &af);
    }

    if (IS_AFFECTED(mob, AFF_PROTECT_GOOD)) {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type = gsn_protection_good;
        af.level = mob->getRealLevel();
        af.duration = -1;
        af.location = APPLY_SAVES;
        af.modifier = -1;
        af.bitvector.setValue(AFF_PROTECT_GOOD);
        affect_to_char(mob, &af);
    }

    if (IS_AFFECTED(mob, AFF_CORRUPTION) && !mob->isAffected(gsn_corruption)) {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type = gsn_corruption;
        af.level = mob->getRealLevel();
        af.duration = -1;
        af.bitvector.setValue(AFF_CORRUPTION);
        affect_to_char(mob, &af);
    }
}


/*
 * Create an object with modifying the count
 */
Object *create_object(OBJ_INDEX_DATA *pObjIndex, short level)
{
    return create_object_org(pObjIndex, level, true);
}

/*
 * for player load/quit
 * Create an object and do not modify the count
 */
Object *create_object_nocount(OBJ_INDEX_DATA *pObjIndex, short level)
{
    return create_object_org(pObjIndex, level, false);
}

/*
 * Create an instance of an object.
 */
Object *create_object_org(OBJ_INDEX_DATA *pObjIndex, short level, bool Count)
{
    Object *obj;

    if (pObjIndex == 0) {
        bug("Create_object: 0 pObjIndex.", 0);
        exit(1);
    }

    obj = ObjectManager::getObject();

    obj->pIndexData = pObjIndex;
    obj->in_room = 0;
    obj->updateCachedNouns();

    pObjIndex->instances.push_back(obj);

    if (obj->pIndexData->limit != -1 && obj->pIndexData->count > obj->pIndexData->limit)
        LogStream::sendWarning() << "Obj limit exceeded for vnum " << pObjIndex->vnum << endl;

    obj->level = pObjIndex->level;

    obj->item_type = pObjIndex->item_type;
    obj->extra_flags = pObjIndex->extra_flags;
    obj->wear_flags = pObjIndex->wear_flags;
    obj->value0(pObjIndex->value[0]);
    obj->value1(pObjIndex->value[1]);
    obj->value2(pObjIndex->value[2]);
    obj->value3(pObjIndex->value[3]);
    obj->value4(pObjIndex->value[4]);
    obj->weight = pObjIndex->weight;
    obj->extracted = false;
    obj->from = str_dup(""); /* used with body parts */
    obj->condition = pObjIndex->condition;

    if (level == 0)
        if (obj->cost > 1000)
            obj->cost = pObjIndex->cost / 10;
        else
            obj->cost = pObjIndex->cost;

    else
        obj->cost = number_fuzzy(level);

    /*
         * Mess with object properties.
         */
    switch (obj->item_type) {
    case ITEM_LIGHT:
        if (obj->value2() == 999)
            obj->value2(-1);
        break;

    case ITEM_CORPSE_PC:
        obj->value3(ROOM_VNUM_ALTAR);
        break;
    }

    obj_to_list(obj);

    if (Count)
        pObjIndex->count++;

    /* assign behavior */
    if (pObjIndex->behavior)
        ObjectBehaviorManager::assign(obj);
    else
        ObjectBehaviorManager::assignBasic(obj);

    /* fenia objprog initialization */
    if (Count) {
        WrapperBase *w = get_wrapper(pObjIndex->wrapper);
        if (w) {
            static Scripting::IdRef initId("init");
            try {
                w->call(initId, "O", obj);
            } catch (const Exception &e) {
                LogStream::sendError()
                    << "create_object #" << pObjIndex->vnum
                    << ": " << e.what() << endl;
            }
        }
    }

    // Notify item creation listeners.
    eventBus->publish(ItemCreatedEvent(obj, Count));
    return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(Object *parent, Object *clone)
{
    int i;

    if (parent == 0 || clone == 0)
        return;

    /* start fixing the object */
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;

        clone->setKeyword(parent->getRealKeyword(lang), lang);
        clone->setShortDescr(parent->getRealShortDescr(lang), lang);
        clone->setDescription(parent->getRealDescription(lang), lang);
    }
        
    if (parent->getRealMaterial())
        clone->setMaterial(parent->getMaterial());
    clone->setOwner(parent->getOwner());

    clone->item_type = parent->item_type;
    clone->extra_flags = parent->extra_flags;
    clone->wear_flags = parent->wear_flags;
    clone->weight = parent->weight;
    clone->cost = parent->cost;
    clone->level = parent->level;
    clone->condition = parent->condition;

    clone->pocket = parent->pocket;
    clone->timer = parent->timer;
    clone->from = str_dup(parent->from);
    clone->extracted = parent->extracted;

    for (i = 0; i < 5; i++)
        clone->valueByIndex(i, parent->valueByIndex(i));

    /* affects */

    for (auto &paf : parent->affected)
        affect_to_obj(clone, paf);

    JsonUtils::copy(clone->props, parent->props);
}

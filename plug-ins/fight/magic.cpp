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

#include <cmath>
#include <vector>
#include <jsoncpp/json/json.h>
#include "configurable.h"

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
#include "rageoath.h"
#include "clanreference.h"
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "loadsave.h"
#include "interp.h"
#include "vnum.h"
#include "fight_exception.h"
#include "damage.h"
#include "immunity.h"
#include "material.h"
#include "fight.h"
#include "damage.h"
#include "def.h"
#include "l10n.h"
#include "damageflags.h"
#include "dl_math.h"

CLAN(none);
GSN(deafen);
GSN(garble);
GSN(holy_craft);
GSN(shielding);
GSN(spell_craft);

/* A multi-hit spell (magic missile) rolls a save per hit: show the
 * immune/resistant/vulnerable hint once per cast and victim, not per hit. */
static bool save_hint_once( Character *victim )
{
    static long lastSerial = -1;
    static Character *lastVictim = 0;

    if (fightspam_cast_depth <= 0 && save_hint_scope <= 0)
        return true;

    if (lastSerial == fightspam_cast_serial && lastVictim == victim)
        return false;

    lastSerial = fightspam_cast_serial;
    lastVictim = victim;
    return true;
}

/*
 * Saving throw models, fight/saves.json.
 *
 * Model 0: the legacy linear roll, kept bit for bit.
 * Model 1: a saturating curve. Every point of svs adds a little less than the one before,
 *          the chance never reaches the ceiling, and the level gap is added outside the curve,
 *          so cast level always cuts through svs:
 *          P = clamp(base + span * (1 - e^-r) + per_level * (Lv - Lc) + shifts, floor, ceiling),
 *          r = svs / (tau * max(level_floor, Lv)), svs = -saving_throw.
 *          A cursed victim (saving_throw > 0) keeps the legacy linear penalty instead of the
 *          exponential, which would fall off faster than today.
 *
 * A missing file, a missing "model" key, or any missing model 1 knob means model 0.
 * Chances are permille everywhere: model 0 rolls number_percent() < save, which is
 * exactly (save - 1) * 10 permille.
 */
struct saves_config_t {
    saves_config_t() : model(0), curveValid(false), dispelValid(false) { }

    int model;
    bool curveValid;
    bool dispelValid;

    double base, span, perLevel, floor, ceiling, tau, levelFloor;
    int shiftBase, shiftPerLevelDiv;
    int charmPerCha, charmLowCha, charmLowChaBase, charmLowChaPerLevelDiv;

    int dispelBase, dispelPerLevel, dispelMin, dispelMax, dispelOwnDebuffPct;
};

static saves_config_t savesConfig;

static bool saves_json_number(const Json::Value &obj, const char *key, double &out)
{
    if (!obj.isObject() || !obj.isMember(key) || !obj[key].isNumeric())
        return false;

    out = obj[key].asDouble();
    return true;
}

static bool saves_json_int(const Json::Value &obj, const char *key, int &out)
{
    double d;
    if (!saves_json_number(obj, key, d))
        return false;

    out = (int)d;
    return true;
}

CONFIGURABLE_LOADED(fight, saves)
{
    saves_config_t cfg;
    static const Json::Value nullValue;
    const Json::Value &shift = value.isObject() && value.isMember("shift") ? value["shift"] : nullValue;
    const Json::Value &charm = value.isObject() && value.isMember("charm") ? value["charm"] : nullValue;
    const Json::Value &dispel = value.isObject() && value.isMember("dispel") ? value["dispel"] : nullValue;

    cfg.curveValid = saves_json_number(value, "base", cfg.base)
        && saves_json_number(value, "span", cfg.span)
        && saves_json_number(value, "per_level", cfg.perLevel)
        && saves_json_number(value, "floor", cfg.floor)
        && saves_json_number(value, "ceiling", cfg.ceiling)
        && saves_json_number(value, "tau", cfg.tau)
        && saves_json_number(value, "level_floor", cfg.levelFloor)
        && saves_json_int(shift, "base", cfg.shiftBase)
        && saves_json_int(shift, "per_level_div", cfg.shiftPerLevelDiv)
        && saves_json_int(charm, "per_cha", cfg.charmPerCha)
        && saves_json_int(charm, "low_cha", cfg.charmLowCha)
        && saves_json_int(charm, "low_cha_pc_base", cfg.charmLowChaBase)
        && saves_json_int(charm, "low_cha_pc_per_level_div", cfg.charmLowChaPerLevelDiv)
        && cfg.span >= 0 && cfg.tau > 0 && cfg.levelFloor > 0
        && cfg.shiftPerLevelDiv > 0 && cfg.charmLowChaPerLevelDiv > 0
        && 0 <= cfg.floor && cfg.floor <= cfg.ceiling && cfg.ceiling <= 100;

    cfg.dispelValid = saves_json_int(dispel, "base", cfg.dispelBase)
        && saves_json_int(dispel, "per_level", cfg.dispelPerLevel)
        && saves_json_int(dispel, "min", cfg.dispelMin)
        && saves_json_int(dispel, "max", cfg.dispelMax)
        && saves_json_int(dispel, "own_debuff_pct", cfg.dispelOwnDebuffPct)
        && 0 <= cfg.dispelMin && cfg.dispelMin <= cfg.dispelMax && cfg.dispelMax <= 100
        && 0 <= cfg.dispelOwnDebuffPct && cfg.dispelOwnDebuffPct <= 100;

    int model = 0;
    if (saves_json_int(value, "model", model) && model == 1) {
        if (!cfg.curveValid)
            LogStream::sendError() << "fight/saves: model 1 asked but a curve knob is missing or out of range, using model 0" << endl;
        else {
            cfg.model = 1;

            if (!cfg.dispelValid)
                LogStream::sendError() << "fight/saves: dispel block missing or out of range, savesDispel stays legacy" << endl;
        }
    }

    savesConfig = cfg;
}

int saves_model( )
{
    return savesConfig.model;
}

/** Model 0 clamp and permille: number_percent() < save lands for save - 1 rolls out of 100. */
static int saves_legacy_permille( int save )
{
    save = URANGE( 5, save, 95 );
    return (save - 1) * 10;
}

/** Fixed model 1 shift for resist, berserk, vulnerable and holy craft, in percent. */
static int saves_shift( int mlevel )
{
    return savesConfig.shiftBase + mlevel / savesConfig.shiftPerLevelDiv;
}

/** Model 1 save chance before the clamp, in percent. svs: positive is better (-saving_throw). */
static double saves_curve( int victimLevel, int castLevel, int svs )
{
    const saves_config_t &c = savesConfig;
    double svsPart;

    if (svs >= 0)
        svsPart = c.span * (1.0 - exp( -svs / (c.tau * max( c.levelFloor, (double)victimLevel )) ));
    else
        // Cursed victim: the legacy term, integer division included, so curse and the like
        // hurt exactly as much as under model 0.
        svsPart = (svs * 90) / max( 45, victimLevel );

    // Anything under -1000% is the floor anyway, keep it in int range for the hooks.
    return max( -1000.0, c.base + svsPart + c.perLevel * (victimLevel - castLevel) );
}

static int saves_clamp_permille( double percent )
{
    percent = max( savesConfig.floor, min( savesConfig.ceiling, percent ) );
    return (int)lround( percent * 10 );
}

int saves_chance_at( int victimLevel, int castLevel, int saving_throw, int shift, int model )
{
    if (model == 1 && savesConfig.curveValid)
        return saves_clamp_permille( saves_curve( victimLevel, castLevel, -saving_throw ) + shift );

    int save = 40 + (victimLevel - castLevel) * 4
        - (saving_throw * 90) / max( 45, victimLevel ) + shift;
    return saves_legacy_permille( save );
}

int saves_chance( short level, Character *victim, int dam_type, Character *ch, bitstring_t dam_flag, bool holyCraft )
{
    bool model1 = saves_model( ) == 1;
    int mlevel = victim->getModifyLevel( );
    int resist = immune_check( victim, dam_type, dam_flag );

    if (resist == RESIST_IMMUNE)
        return 1000;

    // Legacy shifts are mlevel/5, model 1 shifts are shift_base + mlevel/per_level_div.
    int shift = model1 ? saves_shift( mlevel ) : mlevel / 5;
    int save = 0;

    if (!model1)
        save = 40 + (mlevel - level) * 4 -
            (victim->saving_throw * 90) / max( 45, mlevel );

    if (IS_AFFECTED(victim,AFF_BERSERK))
        save += shift;

    if (resist == RESIST_RESISTANT)
        save += shift;
    else if (resist == RESIST_VULNERABLE)
        save -= shift;

    if (holyCraft)
        save -= shift;

    if (dam_type == DAM_CHARM) {
        int cha_vict = victim->getCurrStat(STAT_CHA);
        int cha_diff = ch ? ch->getCurrStat(STAT_CHA) - cha_vict : 0;

        if (model1) {
            save -= cha_diff * savesConfig.charmPerCha;

            if (!victim->is_npc( ) && cha_vict < savesConfig.charmLowCha)
                save -= savesConfig.charmLowChaBase + mlevel / savesConfig.charmLowChaPerLevelDiv;
        }
        else {
            save -= cha_diff * 2;

            if (!victim->is_npc( ) && cha_vict < 18)
                save -= mlevel / 5;
        }
    }

    // Model 1: the shifts so far ride on the curve. Caster hooks see the running chance
    // in percent either way and their edit is kept as a shift.
    double curve = model1 ? saves_curve( mlevel, level, -victim->saving_throw ) : 0;
    int running = model1 ? (int)lround( curve ) + save : save;
    int before = running;

    if (ch)
        for (auto &paf: ch->affected.findAllWithHandler())
            if (paf->type->getAffect())
                paf->type->getAffect( )->onSaves(
                    SpellTarget::Pointer(NEW, ch), paf, victim, running, dam_type);

    if (!model1)
        return saves_legacy_permille( running );

    return saves_clamp_permille( curve + save + (running - before) );
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 * The hint and holy craft rolls run before the chance, in the legacy order.
 */
bool saves_spell( short level, Character *victim, int dam_type, Character *ch, bitstring_t dam_flag, bool verbose )
{
    int resist = immune_check(victim, dam_type, dam_flag);

    // Model 1 passes DAMF_MAGIC/DAMF_PRAYER on every spell save, so res/vuln spell can be
    // what fires the hint. Then the damage noun would be wrong: name magic instead.
    bool byMagic = saves_model( ) == 1
        && (resist == RESIST_RESISTANT || resist == RESIST_VULNERABLE)
        && immune_check(victim, dam_type, dam_flag & ~(DAMF_MAGIC|DAMF_PRAYER|DAMF_SPELL)) != resist;

    switch(resist) {
        case RESIST_IMMUNE:
            if (ch && verbose && save_hint_once( victim )) {
                if (ch != victim)
                    ch->pecho(_("%^N1, похоже, никак не сможет навредить %C3."), damage_noun(dam_type, viewerLang(ch)).c_str(), victim);
                else
                    ch->pecho(_("%^N1, похоже, никак не сможет навредить тебе."), damage_noun(dam_type, viewerLang(ch)).c_str());
            }
            return true;
        case RESIST_RESISTANT:
            if (ch && verbose && number_percent( ) < gsn_spell_craft->getEffective( ch )
                && save_hint_once( victim )) {
                if (byMagic) {
                    if (ch != victim)
                        ch->pecho(_("Магия {1{Gочень слабо{2 влияет на %C4."), victim);
                    else
                        ch->pecho(_("Магия {1{Gочень слабо{2 влияет на тебя."));
                }
                else if (ch != victim)
                    ch->pecho(_("%^N1 {1{Gочень слабо{2 влияет на %C4."), 
                        damage_noun(dam_type, viewerLang(ch)).c_str(), victim);
                else 
                    ch->pecho(_("%^N1 {1{Gочень слабо{2 влияет на тебя."), 
                        damage_noun(dam_type, viewerLang(ch)).c_str());
            }
            break;
        case RESIST_VULNERABLE:
            if (ch && verbose && number_percent( ) < gsn_spell_craft->getEffective( ch )
                && save_hint_once( victim )) {
                if (byMagic) {
                    if (ch != victim)
                        ch->pecho(_("Магия {1{Rособо пагубно{2 влияет на %C4."), victim);
                    else
                        ch->pecho(_("Магия {1{Rособо пагубно{2 влияет на тебя."));
                }
                else if (ch != victim)
                    ch->pecho(_("%^N1 {1{Rособо пагубно{2 влияет на %C4."), 
                        damage_noun(dam_type, viewerLang(ch)).c_str(), victim);
                else
                    ch->pecho(_("%^N1 {1{Rособо пагубно{2 влияет на тебя."), 
                        damage_noun(dam_type, viewerLang(ch)).c_str());
            }
            break;
    }

    bool holyCraft = false;

    if (ch && IS_GOOD(ch) && IS_EVIL(victim))
        if (number_percent( ) < gsn_holy_craft->getEffective( ch )) {
            holyCraft = true;
            gsn_holy_craft->improve( ch, true );
        }

    int chance = saves_chance( level, victim, dam_type, ch, dam_flag, holyCraft );

    if (saves_model( ) == 1)
        return number_range( 1, 1000 ) <= chance;

    return number_percent( ) * 10 <= chance;
}

/*
 * Fenia spell scripts in flight. A Fenia vict.saves_spell() call has no skill of its own,
 * so under model 1 it takes DAMF_MAGIC or DAMF_PRAYER from the innermost running spell,
 * decided the way savesSpell() decides it. A value stack, not a pointer chain into stack
 * frames, so a scheduler thread that outlives its spell can never leave a dangling top.
 */
static std::vector<bitstring_t> savesSpellFlags;

SavesSpellScope::SavesSpellScope( Spell *spell, Character *caster )
{
    bool prayer = spell && caster && spell->isPrayer( caster );
    savesSpellFlags.push_back( prayer ? DAMF_PRAYER : DAMF_MAGIC );
}

SavesSpellScope::~SavesSpellScope( )
{
    if (!savesSpellFlags.empty( ))
        savesSpellFlags.pop_back( );
}

bitstring_t saves_default_damflag( )
{
    if (saves_model( ) != 1 || savesSpellFlags.empty( ))
        return DAMF_OTHER;

    return savesSpellFlags.back( );
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
            ch->pecho(_("Ты пытаешься сотворить заклинание, но изолирующий экран блокирует тебя."));
            ch->recho(_("%1$^C1 пыта%1$nется|ются сотворить заклинание, но изолирующий экран блокирует %1$P2."), ch);
            return false;
        }

        if (ch->isAffected( gsn_garble ) && number_percent( ) > 50) {
            ch->pecho(_("Твой язык заплетается, и ты не можешь как следует произнести заклинание."));
            ch->recho(_("Язык %1$C2 заплетается, и %1$P1 не мо%1$nжет|гут как следует произнести заклинание."), ch);        
            return false;
        }
        
        if (ch->isAffected( gsn_deafen ) && number_percent( ) > 50) {
            ch->pecho(_("Глухота мешает тебе как следует произнести заклинание."));
            ch->recho(_("Глухота мешает %1$C3 как следует произнести заклинание."), ch);        
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
        ch->pecho(_("Эта местность поглощает и блокирует твое заклинание."));
        ch->recho(_("%1$^C1 пыта%1$nется|ются сотворить заклинание, но что-то в этой местности блокирует %1$P2."), ch);
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
        spell->utter( ch, target->victim );

    // The caster's own lag: mob AI casts here without going through interpret.
    if (IS_SET(flags, FSPELL_WAIT)) {
        InterpretActorGuard guard( ch );
        ch->setWait( spell->getBeats(ch) );
    }

    if (IS_SET(flags, FSPELL_CHECK_SAFE) && offensive && target->victim) {
        if (is_safe_attack(ch, target->victim))
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
void spell_by_item( Character *ch, Object *obj, bool oathPaid, bool burned )
{
    for (int i = 1; i <= 4; i++) {
        Skill *skill;
        Spell::Pointer spell;
        SpellTarget::Pointer result( NEW );
        int target;

        // Slot 0 is the "none" skill: an empty slot, not a bad spell.
        if (obj->valueByIndex(i) <= 0)
            continue;

        skill = SkillManager::getThis( )->find( obj->valueByIndex(i) );

        if (!skill)
            continue;

        if (burned && !rage_natural_spell( skill->getName( ) ))
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
        

        // A Battlerager's own potion already paid the oath (rage_own_magic at quaff/eat).
        bool ownOath = oathPaid && (result->victim == ch || result->type == SpellTarget::ROOM);

        try {
            if (ownOath || !spell->spellbane( ch, result->victim )) {
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

/*
 * Model 1 (fight/saves.json "dispel"): base + per_level per level of difference, clipped
 * to min..max. An own debuff is an affect whose sources list the dispeller, sitting on
 * somebody else: e.g. dispel affects cast on an enemy you put to sleep. Only the dispel
 * part of the chance is scaled by own_debuff_pct, so your own debuffs come off less often.
 * Fenia affects record a source only when the script sets af.source.
 */
bool savesDispel( int dis_level, int spell_level, int duration, bool ownDebuff )
{
    int save;

    /* impossible to dispel permanent effects */
    if (duration == -2 ) return 1;
    if (duration == -1 ) spell_level += 5;

    if (saves_model( ) == 1 && savesConfig.dispelValid) {
        const saves_config_t &c = savesConfig;

        save = c.dispelBase + (spell_level - dis_level) * c.dispelPerLevel;
        save = URANGE( c.dispelMin, save, c.dispelMax );

        if (ownDebuff)
            save = 100 - (100 - save) * c.dispelOwnDebuffPct / 100;

        return number_range( 1, 100 ) <= save;
    }

    save = 50 + (spell_level - dis_level) * 5;
    save = URANGE( 5, save, 95 );
    return number_percent( ) < save;
}

bool checkDispel( int dis_level, Character *victim, int sn, Character *dispeller )
{
    for (auto af: victim->affected.clone()) {
        if (af->type != sn) 
            continue;

        bool ownDebuff = dispeller && dispeller != victim
            && af->sources.contains( AffectSource( dispeller ) );

        if (!savesDispel(dis_level,af->level,af->duration,ownDebuff)) {
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

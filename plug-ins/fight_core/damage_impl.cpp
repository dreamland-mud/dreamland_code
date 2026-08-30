/* $Id: damage_impl.cpp,v 1.1.2.6 2009/11/08 17:46:27 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "damage_impl.h"
#include "damageflags.h"

#include "logstream.h"
#include "inflectedstring.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "affect.h"
#include "affecthandler.h"
#include "skillreference.h"
#include "spell.h"
#include "skillgroup.h"
#include "npcharacter.h"
#include "configurable.h"
#include "multiinflectedstring.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

#include <map>

GSN(resistance);
GSN(mental_knife);
GSN(dragons_breath);
GROUP(draconian);

/*-----------------------------------------------------------------------------
 * Self Damage
 *---------------------------------------------------------------------------*/
SelfDamage::SelfDamage( Character *ch, int dam_type, int dam ) : Damage( ch, ch, dam_type, dam )
{
}

void SelfDamage::calcDamage( )
{
    protectRazer( );
}
    
/*-----------------------------------------------------------------------------
 * Raw Damage
 *---------------------------------------------------------------------------*/
RawDamage::RawDamage( Character *ch, Character *victim, int dam_type, int dam, const DLString &deathReason )
    : Damage( ch, victim, dam_type, dam )
{
    this->deathReason = deathReason;
}

void RawDamage::message( )
{
    if( ch == victim ) {
        msgRoom( _("%2$^C1\6себя"), dam, ch );
        msgChar( _("Ты\5себя"), dam );
        return;
    }

    if ( dam == 0 ) {
       msgRoom( "%2$^C1\6%3$C2", dam, ch, victim);
       msgChar( _("Ты\5%3$C2"), dam, ch, victim);
    }
    else {
        msgRoom( "%2$^C1\6%3$C4", dam, ch, victim );
        msgChar( _("Ты\5%3$C4"), dam, ch, victim );
    }

    msgVict( _("%2$^C1\6тебя"), dam, ch );
}

bool RawDamage::canDamage( )
{
    return true;
}

/*-----------------------------------------------------------------------------
 * SkillDamage 
 *----------------------------------------------------------------------------*/
SkillDamage::SkillDamage( Character *ch, Character *victim, 
                          int sn, int dam_type, int dam, bitstring_t dam_flag )
            : Damage( ch, victim, dam_type, dam, dam_flag )
{
    this->sn = sn;
    this->deathReason = skillManager->find(sn)->getName();
}

SkillDamage::SkillDamage( Affect *paf, Character *victim, int sn, int dam_type, int dam, bitstring_t dam_flag )
            : Damage( paf, victim, dam_type, dam, dam_flag )
{
    this->sn = sn;
    this->deathReason = skillManager->find(sn)->getName();
}

int SkillDamage::msgNoSpamBit( )
{
    return CONFIG_SKILLSPAM;
}

/*-----------------------------------------------------------------------------
 * Trilingual skill damage nouns (Trello 2594)
 *
 * A skill's damage noun (dammsg, e.g. "струя кислоты") is stored RU-only as an
 * XMLInflectedString. The en/ua forms live in config/fight/skill_dammsg.json,
 * keyed by the RU fullForm; a skill with no entry falls back to RU. Same engine
 * (MultiInflectedString) as the attack_table nouns in onehit_undef.cpp.
 *----------------------------------------------------------------------------*/
static std::map<DLString, std::pair<DLString, DLString> > skillDammsg;

CONFIGURABLE_LOADED(fight, skill_dammsg)
{
    skillDammsg.clear();
    Json::Value::Members keys = value.getMemberNames();
    for (Json::Value::Members::const_iterator k = keys.begin(); k != keys.end(); k++) {
        const Json::Value &v = value[*k];
        skillDammsg[DLString(k->c_str())] = std::make_pair(
            DLString(v["en"].asString().c_str()), DLString(v["ua"].asString().c_str()));
    }
}

/*-----------------------------------------------------------------------------
 * Trilingual damage-class nouns (Trello AXqNSTVz)
 *
 * damage_table (damageflags.conf) keeps the RU inflected noun byte-identical;
 * the en/ua nominative forms live in config/fight/damage_nouns.json, indexed
 * 1:1 with the enumeration. The spell immune/resist lines in magic.cpp route
 * the noun through damage_noun() so a non-RU viewer sees the damage type in
 * their own language instead of the Russian fallback.
 *----------------------------------------------------------------------------*/
struct DamageNounRow {
    DLString en, ua;
    void fromJson(const Json::Value &v)
    {
        en = v["en"].asString();
        ua = v["ua"].asString();
    }
};
static json_vector<DamageNounRow> damageNouns;

CONFIGURABLE_LOADED(fight, damage_nouns)
{
    damageNouns.fromJson(value);
}

DLString damage_noun(int dam_type, lang_t lang)
{
    if (dam_type >= 0 && dam_type < (int)damageNouns.size()) {
        if (lang == LANG_EN && !damageNouns[dam_type].en.empty())
            return damageNouns[dam_type].en;
        if (lang == LANG_UA && !damageNouns[dam_type].ua.empty())
            return damageNouns[dam_type].ua;
    }

    // RU, or a missing translation: fall back to the inflected RU noun.
    return damage_table.message(dam_type);
}

/*-----------------------------------------------------------------------------
 * Combat spell-proc values (gear advisor)
 *
 * config/fight/spell_combat_value.json maps a spell name to its relative combat
 * worth (0-100). The gear advisor's ga_score reads an item's <props>combatcast</props>
 * and multiplies these values by proc chance, cast count, an item-level scale and a
 * global weight to replace the flat +50 those items used to get. Damage spells are
 * Fenia (C++ can't introspect their damage), so the table is modeled, not computed.
 * Keys starting with '_' are knobs/meta, not spells: _global scales procs against
 * stats, _level_ref is the item level that scores at 1.0x. COMBAT_PROC_SCORING.md.
 *----------------------------------------------------------------------------*/
static std::map<DLString, double> spellCombatValue;
static double spellComboGlobal   = 1.0;
static double spellComboLevelRef = 50.0;

CONFIGURABLE_LOADED(fight, spell_combat_value)
{
    spellCombatValue.clear();
    spellComboGlobal   = 1.0;
    spellComboLevelRef = 50.0;

    for (auto i = value.begin(); i != value.end(); ++i) {
        DLString key = i.key().asString();
        if (key.empty())
            continue;
        if (key.at(0) == '_') {
            if (key == "_global")    spellComboGlobal   = (*i).asDouble();
            if (key == "_level_ref") spellComboLevelRef = (*i).asDouble();
            continue;
        }
        spellCombatValue[key] = (*i).asDouble();
    }

    // Never divide by zero when scaling by item level.
    if (spellComboLevelRef <= 0)
        spellComboLevelRef = 50.0;
}

double spell_combat_value(const DLString &spell)
{
    std::map<DLString, double>::const_iterator i = spellCombatValue.find(spell);
    return i == spellCombatValue.end() ? 0.0 : i->second;
}

double spell_combat_global()    { return spellComboGlobal; }
double spell_combat_level_ref() { return spellComboLevelRef; }

void SkillDamage::message( )
{
    const InflectedString &dammsg = skillManager->find(sn)->getDammsg( );

    DLString en, ua;
    std::map<DLString, std::pair<DLString, DLString> >::const_iterator i = skillDammsg.find(dammsg.getFullForm());
    if (i != skillDammsg.end()) {
        en = i->second.first;
        ua = i->second.second;
    }
    MultiInflectedString attack(dammsg.getFullForm(), en, ua, dammsg.getMultiGender());

    if (immune) {
        if (ch == victim) {
            msgRoom(_("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %3$P4 сам%3$Gого|ого|ой|их"), dam, &attack, ch);
            msgChar(_("Тебе повезло, у тебя иммунитет к этому"), dam);
        }
        else {
            msgRoom(_("%2$^O1 %3$C2 бессил%2$Gьно|ен|ьна|ьны против %4$C2"), dam, &attack, ch, victim);
            msgChar(_("%2$^T1 %2$O1 бессил%2$Gьно|ен|ьна|ьны против %3$C2"), dam, &attack, victim);
            msgVict(_("Против тебя %3$O1 %2$C2 бессил%3$Gьно|ен|ьна|ьны"), dam, ch, &attack);
        }
    }
    else {
        if (ch == victim) {
            msgRoom( _("%2$^O1 %3$C2\6%3$P2"), dam, &attack, ch );
            msgChar( _("%2$^T1 %2$O1\6тебя"), dam, &attack );
        }
        else {
            if ( dam == 0 )
            {
                msgRoom( _("%2$^O1 %3$C2\6%4$C2"), dam, &attack, ch, victim );
                msgChar( _("%2$^T1 %2$O1\6%3$C2"), dam, &attack, victim );
            }
            else {
                msgRoom( _("%2$^O1 %3$C2\6%4$C4"), dam, &attack, ch, victim );
                msgChar( _("%2$^T1 %2$O1\6%3$C4"), dam, &attack, victim );
            }

            msgVict( _("%2$^O1 %3$C2\6тебя"), dam, &attack, ch );
        }
    }
}


bool SkillDamage::mprog_immune()
{
    DLString damType = damage_table.name(dam_type);
    DLString sname = skillManager->find(sn)->getName();
    FENIA_NUM_CALL(victim, "Immune", dam, "CisOis", ch, dam, damType.c_str(), NULL, dam_flag, sname.c_str());
    FENIA_NDX_NUM_CALL(victim->getNPC(), "Immune", dam, "CCisOis", victim, ch, dam, damType.c_str(), NULL, dam_flag, sname.c_str());

    for (auto &paf: victim->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect()->onImmune(SpellTarget::Pointer(NEW, victim), paf, ch, dam, damType.c_str(), NULL, dam_flag, sname.c_str()))
            return true;

    return false; 
}

/*
 * 'resistance' reduces 50% of non-magical damage,
 * with the historical exception for 'mental knife' spell
 */
void SkillDamage::protectResistance( )
{
    if (!victim->isAffected(gsn_resistance))
        return;

    if (sn == gsn_mental_knife) {
        dam -= ( dam * 2 / 5 );
        return;
    }

    if (sn == gsn_dragons_breath) {
        return;
    }

    Skill *skill = skillManager->find( sn );
    Spell::Pointer spell = skill->getSpell( );

    if (!spell 
            || !spell->isCasted( ) 
            || spell->isPrayer( ch )
            || skill->hasGroup(group_draconian))
    {
        dam -= ( dam / 2 );
        return;
    }
}


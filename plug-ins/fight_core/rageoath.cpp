#include "rageoath.h"
#include "clanownshook.h"
#include "damage_impl.h"
#include "damageflags.h"
#include "fight_safe.h"

#include "skillreference.h"
#include "skillgroup.h"
#include "spell.h"
#include "skill.h"
#include "clanreference.h"
#include "profession.h"
#include "religion.h"
#include "pcharacter.h"
#include "act.h"
#include "l10n.h"
#include "merc.h"
#include "def.h"

CLAN(battlerager);
PROF(cleric);
PROF(paladin);
RELIG(turlok);
RELIG(ares);
RELIG(alala);
RELIG(goktengri);
GSN(spellbane);
GROUP(arcane);
GROUP(transportation);

/** Zealotry by rank when the clan plugin is unloaded; the clan XML rankPower is the live table. */
static const int DEFAULT_ZEAL[] = { 30, 37, 44, 51, 58, 65, 72, 79, 86 };
static const int RANK_COUNT = sizeof( DEFAULT_ZEAL ) / sizeof( *DEFAULT_ZEAL );
/** A spellbane outside the clan (a mob, a borrowed skill) keeps the old flat 2/3 for players. */
static const int OUTSIDER_ZEAL = 67;
/** Offensive prayers start to bounce off at this rank. */
static const int PRAYER_DEFLECT_RANK = 3;
/** Highest rank that receives heals, and cures. */
static const int HEALS_MAX_RANK = 2;
static const int CURES_MAX_RANK = 5;

static const char * const HEALS[] = {
    "heal", "superior heal", "master healing", "group heal", "refresh", "frenzy", 0
};
static const char * const CURES[] = {
    "cure poison", "cure disease", "cure blindness", 0
};
/** The Rage gods' own voice: every rank accepts it. */
static const char * const HYMN = "hymn of rage";
/** Prayers a divine member may cast besides the heal and cure lists. */
static const char * const DIVINE_EXTRA[] = {
    "blade barrier", "hymn of rage", 0
};

static bool listed( const char * const *list, const DLString &name )
{
    for (int i = 0; list[i]; i++)
        if (name == list[i])
            return true;
    return false;
}

bool rage_member( Character *ch )
{
    return ch && !ch->is_npc( ) && !ch->is_immortal( ) && ch->getClan( ) == clan_battlerager;
}

int rage_rank( Character *ch )
{
    if (!rage_member( ch ))
        return -1;
    return URANGE( 0, (int)ch->getPC( )->getClanLevel( ), RANK_COUNT - 1 );
}

bool rage_divine( Character *ch )
{
    return rage_member( ch )
           && (ch->getProfession( ) == prof_cleric || ch->getProfession( ) == prof_paladin);
}

bool rage_god( Character *ch )
{
    return ch->getReligion( ) == god_turlok || ch->getReligion( ) == god_ares
           || ch->getReligion( ) == god_alala || ch->getReligion( ) == god_goktengri;
}

int rage_zeal( Character *ch )
{
    int rank = rage_rank( ch );
    if (rank < 0)
        return OUTSIDER_ZEAL;

    int power = clan_char_power( ch );
    return power >= 0 ? power : DEFAULT_ZEAL[rank];
}

int rage_deflect( Character *ch )
{
    int eff = gsn_spellbane->getEffective( ch );
    if (ch->is_npc( ))
        return eff;
    return eff * rage_zeal( ch ) / 100;
}

int rage_prayer_deflect( Character *ch )
{
    if (rage_rank( ch ) < PRAYER_DEFLECT_RANK)
        return 0;
    return rage_deflect( ch ) / 2;
}

bool rage_prayer_accepted( Character *caster, Character *vch, const DLString &skillName )
{
    if (vch->is_npc( ))
        return false;

    if (skillName == HYMN)
        return true;

    int rank = rage_rank( vch );
    if (rank < 0 || !rage_god( caster ))
        return false;

    if (listed( HEALS, skillName ))
        return rank <= HEALS_MAX_RANK;

    if (listed( CURES, skillName ))
        return rank <= CURES_MAX_RANK;

    return false;
}

bool rage_cast_allowed( Character *ch, const DLString &skillName )
{
    return rage_divine( ch )
           && (listed( HEALS, skillName ) || listed( CURES, skillName ) || listed( DIVINE_EXTRA, skillName ));
}

bool rage_skill_forbidden( Character *ch, Skill *skill )
{
    if (!rage_member( ch ))
        return false;

    if (skill->getSpell( ) && skill->getSpell( )->isCasted( ))
        return !rage_cast_allowed( ch, skill->getName( ) );

    return skill->hasGroup( group_arcane ) || skill->hasGroup( group_transportation );
}

bool rage_magic_allowed( Character *ch, RageMagic kind )
{
    int rank = rage_rank( ch );
    if (rank < 0)
        return true;

    switch (kind) {
    case RAGE_POTION:   return rank <= 5;
    case RAGE_PORTAL:   return rank <= 2;
    case RAGE_LANGUAGE: return rank <= 2;
    case RAGE_CARDS:    return rank <= 5;
    }

    return false;
}

void rage_magic_refuse( Character *ch )
{
    ch->pecho( _("Твоя клятва ярости слишком глубока: эта магия тебе больше не дозволена.") );
}

bool rage_own_magic( Character *ch )
{
    bool burned = number_percent( ) <= rage_deflect( ch );
    int dam = ch->max_hit / (burned ? 10 : 20);

    if (burned) {
        oldact( _("Аура ярости выжигает эту магию дотла, и боль скручивает тебя!"), ch, 0, 0, TO_CHAR );
        oldact( _("Аура ярости $c2 вспыхивает и выжигает магию дотла."), ch, 0, 0, TO_ROOM );
    }
    else {
        oldact( _("Клятва ярости напоминает о себе острой болью."), ch, 0, 0, TO_CHAR );
    }

    ch->hit = max( 1, ch->hit - dam );
    return burned;
}

bool rage_area_bane( Character *caster, Character *vch, bool prayer, bool retaliate )
{
    if (!vch->isAffected( gsn_spellbane ))
        return false;

    int chance = (prayer ? rage_prayer_deflect( vch ) : rage_deflect( vch )) / 2;
    if (chance <= 0 || number_percent( ) > chance)
        return false;

    oldact( _("Твоя аура ярости отводит от тебя это колдовство."), vch, 0, 0, TO_CHAR );
    oldact( _("Аура ярости $c2 отводит колдовство в сторону."), vch, 0, 0, TO_ROOM );

    if (retaliate && !prayer && caster && caster != vch
        && !is_safe_nomessage( vch, caster ) && !is_safe_nomessage( caster, vch ))
    {
        gsn_spellbane->improve( vch, true, caster );
        SkillDamage( vch, caster, gsn_spellbane, DAM_NEGATIVE, vch->getModifyLevel( ), DAMF_MAGIC ).hit( true );
    }

    return true;
}

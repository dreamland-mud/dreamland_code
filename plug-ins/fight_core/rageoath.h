#ifndef RAGEOATH_H
#define RAGEOATH_H

class Character;
class Skill;
class DLString;

/*
 * The Battlerager oath (docs/systems/BATTLERAGER_REFORM.md §6).
 * Devotion grows with clan rank: the aura of rage (skill 'spellbane') deflects more,
 * and the member loses what little magic the oath still tolerated at low ranks.
 */

/** Magic a low-rank member may still touch, each one up to its own highest rank. */
enum RageMagic {
    RAGE_POTION,    // quaff, eat pills
    RAGE_PORTAL,    // walk through a magic portal
    RAGE_LANGUAGE,  // utter a word of an ancient language
    RAGE_CARDS      // skills of the card deck
};

/** A mortal player of the Battlerager clan. */
bool rage_member( Character *ch );
/** Clan rank 0-8 of a member, -1 for everyone else. */
int rage_rank( Character *ch );
/** A member of a divine class (cleric, paladin). */
bool rage_divine( Character *ch );
/** Worships one of the four Rage gods. */
bool rage_god( Character *ch );

/** Zealotry in percent: the clan's rankPower for the member's rank. */
int rage_zeal( Character *ch );
/** Chance to deflect targeted magic: spellbane effective % scaled by zealotry. */
int rage_deflect( Character *ch );
/** Chance to deflect a targeted offensive prayer: half of rage_deflect, from rank 3 up. */
int rage_prayer_deflect( Character *ch );

/** A beneficial prayer the member may receive from caster (a Rage-god priest, rank-dependent list). */
bool rage_prayer_accepted( Character *caster, Character *vch, const DLString &skillName );
/** A prayer a divine member may still cast. */
bool rage_cast_allowed( Character *ch, const DLString &skillName );
/** Magic-origin skill the member must not see or use (practice, dreams, temporary gifts). */
bool rage_skill_forbidden( Character *ch, Skill *skill );

/** Whether the member's rank still tolerates this kind of magic. True for non-members. */
bool rage_magic_allowed( Character *ch, RageMagic kind );
/** Tell the member the oath forbids this magic now. */
void rage_magic_refuse( Character *ch );
/**
 * The member uses magic himself and the aura rolls against it.
 * True: the magic burns out. Either way it hurts, never lethally.
 */
bool rage_own_magic( Character *ch );
/**
 * Room-wide magic reaches a member: half the targeted chance. On a deflect the member is
 * spared and, when retaliate is set, the caster takes the member's level in damage.
 * quiet: no messages (a lingering room affect rolling every round).
 * May throw VictimDeathException for the caster.
 */
bool rage_area_bane( Character *caster, Character *vch, bool prayer, bool retaliate, bool quiet = false );

#endif

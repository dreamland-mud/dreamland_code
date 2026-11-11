/* $Id: personalquestreward.cpp,v 1.1.2.10.6.3 2008/03/21 22:41:58 rufina Exp $
 *
 * ruffina, 2003
 * logic based on progs from DreamLand 2.0
 */
#include "wrapperbase.h"
#include "feniamanager.h"
#include "reglist.h"
#include "regcontainer.h"

#include "personalquestreward.h"
#include "class.h"
#include "pcharacter.h"
#include "core/object.h"
#include "affect.h"
#include "act.h"
#include "loadsave.h"
#include "skillreference.h"

#include "merc.h"
#include "def.h"
#include "l10n.h"

using namespace Scripting;

GSN(none);

// Apply locations a personal reward may hang, in the order the old
// QuestGirth::equip / QuestRing::equip used, saves last (the bag's), then the
// weapon's class and attack overrides (APPLY_WEAPON_CLASS / _ATTACK, folded at
// read time by get_weapon_class / get_weapon_attack). Membership is only the
// candidate set: Fenia answers null for a location the family does not own.
static const int PERSONAL_APPLIES[] = {
    APPLY_INT, APPLY_WIS, APPLY_CON, APPLY_DEX, APPLY_STR,
    APPLY_AC, APPLY_HIT, APPLY_MANA, APPLY_MOVE,
    APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVES,
    APPLY_WEAPON_CLASS, APPLY_WEAPON_ATTACK, -1
};

// The reward's own affects: permanent and untyped. They are hung with type -1
// and read back from a pfile as skill "none"; the 2003 weapon generator hung
// its hitroll/damroll as gsn_none. A skill-typed or timed affect (an enchant,
// a compound prayer, a poison) is someone else's and is never touched.
static bool personal_own_affect( Affect *paf )
{
    int type = paf->type;
    return paf->duration < 0 && (type < 0 || type == (int)gsn_none);
}

// The value a location reads as without any affect: an override affect equal
// to it is pointless, so a missing one is not hung.
static int personal_neutral_value( ::Object *obj, int loc )
{
    if (loc == APPLY_WEAPON_CLASS && obj->item_type == ITEM_WEAPON)
        return obj->value0( );
    if (loc == APPLY_WEAPON_ATTACK && obj->item_type == ITEM_WEAPON)
        return obj->value3( );
    return 0;
}

void PersonalQuestReward::get( Character *ch )
{
    if (!canEquip( ch ))
        return;

    oldact_p(_("{BМерцающая аура окружает $o4.\n\r{x"), ch, obj, 0, TO_CHAR, POS_SLEEPING);
}

DLString PersonalQuestReward::questFamily( ) const
{
    return DLString::emptyString;
}

int PersonalQuestReward::questTier( ::Object *obj )
{
    return atoi( obj->getProperty( "questTier" ).c_str( ) );
}

/*
 * Call .tmp.questreward.modifier(ch, obj, fam, loc, tier) for one apply
 * location. Returns false (and does not touch 'out') on a null answer, or if
 * the Fenia numbers module is not loaded or throws, so the caller keeps
 * whatever stats the item already had rather than wiping worn gear when a
 * codesource fails to parse.
 */
bool PersonalQuestReward::feniaModifier( Character *ch, const DLString &fam, int loc, int tier, int &out )
{
    if (!FeniaManager::wrapperManager)
        return false;

    static IdRef ID_TMP( "tmp" ), ID_QR( "questreward" ), ID_MOD( "modifier" );

    try {
        Register tmp = *Context::root[ID_TMP];
        Register qr = *tmp[ID_QR];
        Register modFn = *qr[ID_MOD];

        if (modFn.type != Register::FUNCTION)
            return false;

        RegisterList args;
        args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );
        args.push_back( FeniaManager::wrapperManager->getWrapper( obj ) );
        args.push_back( Register( fam ) );
        args.push_back( Register( loc ) );
        args.push_back( Register( tier ) );

        Register rc = modFn.toFunction( )->invoke( qr, args );

        if (rc.type != Register::NUMBER)
            return false;

        out = rc.toNumber( );
        return true;

    } catch (const ::Exception &e) {
        FeniaManager::getThis( )->croak( 0, Register( DLString( "questreward.modifier" ) ), e );
        return false;
    }
}

/*
 * Call .tmp.questreward.shape(ch, obj, fam, tier) once per wear, before the
 * affects: it writes item values (a weapon's dice and flags). Values are not
 * char stats, so writing them here, before the item's affects reach the
 * wearer, is safe. A missing function or a throw leaves the values as they are.
 */
void PersonalQuestReward::feniaShape( Character *ch, const DLString &fam, int tier )
{
    if (!FeniaManager::wrapperManager)
        return;

    static IdRef ID_TMP( "tmp" ), ID_QR( "questreward" ), ID_SHAPE( "shape" );

    try {
        Register tmp = *Context::root[ID_TMP];
        Register qr = *tmp[ID_QR];
        Register shapeFn = *qr[ID_SHAPE];

        if (shapeFn.type != Register::FUNCTION)
            return;

        RegisterList args;
        args.push_back( FeniaManager::wrapperManager->getWrapper( (Character *)ch ) );
        args.push_back( FeniaManager::wrapperManager->getWrapper( obj ) );
        args.push_back( Register( fam ) );
        args.push_back( Register( tier ) );

        shapeFn.toFunction( )->invoke( qr, args );

    } catch (const ::Exception &e) {
        FeniaManager::getThis( )->croak( 0, Register( DLString( "questreward.shape" ) ), e );
    }
}

/*
 * Shared path of every personal reward: stamp the item to the wearer's
 * modified level, let Fenia shape its values, then upsert each candidate
 * location from Fenia. The reward's own affect at a location is set; a missing
 * one is hung when the answer differs from what the item reads without it.
 * Affects are hung unapplied: the wear location is set but the item's affects
 * are not on the wearer yet (DefaultWearlocation applies them after equip).
 * A reward with no family (the base class) does nothing.
 */
void PersonalQuestReward::equip( Character *ch )
{
    DLString fam = questFamily( );
    if (fam.empty( ))
        return;

    short level = ch->getModifyLevel( );
    int tier = questTier( obj );

    obj->level = level;

    feniaShape( ch, fam, tier );

    for (int i = 0; PERSONAL_APPLIES[i] != -1; i++) {
        int loc = PERSONAL_APPLIES[i];
        int mod = 0;

        // Null answer (not this family's location) or Fenia unavailable:
        // leave whatever sits there at its last-good value.
        if (!feniaModifier( ch, fam, loc, tier, mod ))
            continue;

        bool found = false;
        for (auto &paf: obj->affected) {
            if (paf->location != loc || !personal_own_affect( paf ))
                continue;

            paf->level = level;
            // A duplicate own affect would stack: only the first one counts.
            paf->modifier = found ? 0 : mod;
            found = true;
        }

        if (found || mod == personal_neutral_value( obj, loc ))
            continue;

        Affect af;
        af.type = -1;
        af.duration = -1;
        af.level = level;
        af.location = loc;
        af.modifier = mod;
        affect_to_obj_unapplied( obj, &af );
    }
}

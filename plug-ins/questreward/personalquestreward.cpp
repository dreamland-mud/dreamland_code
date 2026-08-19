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

#include "merc.h"
#include "def.h"
#include "l10n.h"

using namespace Scripting;

// Apply locations a girth/ring hangs, in the exact order the old
// QuestGirth::equip / QuestRing::equip used. The membership of this list is
// also the "does this family own the location" test for the re-scale path: an
// affect on a location outside it (a player's enchant on saves, say) is left
// untouched, matching the old switch's `default: return`.
static const int PERSONAL_APPLIES[] = {
    APPLY_INT, APPLY_WIS, APPLY_CON, APPLY_DEX, APPLY_STR,
    APPLY_AC, APPLY_HIT, APPLY_MANA, APPLY_MOVE,
    APPLY_HITROLL, APPLY_DAMROLL, -1
};

static bool personal_owns_location( int loc )
{
    for (int i = 0; PERSONAL_APPLIES[i] != -1; i++)
        if (PERSONAL_APPLIES[i] == loc)
            return true;
    return false;
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
 * Call .tmp.questreward.modifier(ch, fam, loc, tier) for one apply location.
 * Returns false (and does not touch 'out') if the Fenia numbers module is not
 * loaded or throws, so the caller keeps whatever stats the item already had
 * rather than wiping worn gear when a codesource fails to parse.
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
 * Shared girth/ring path: stamp the item to the wearer's modified level and
 * hang the full personal apply list, sourcing every modifier from Fenia. First
 * wear adds the affects; later wears re-scale them in place. QuestWeapon
 * overrides this for its conditional affects plus the weapon generator; a
 * reward with no family (base, QuestBag) does nothing.
 */
void PersonalQuestReward::equip( Character *ch )
{
    DLString fam = questFamily( );
    if (fam.empty( ))
        return;

    short level = ch->getModifyLevel( );
    int tier = questTier( obj );

    obj->level = level;

    if (!obj->affected.empty( )) {
        // Re-scale existing affects to the current level and tier. Only touch
        // locations this family owns, so a player's enchant is left alone. If
        // Fenia is unavailable, leave the affect at its last-good value.
        for (auto &paf: obj->affected) {
            if (!personal_owns_location( paf->location ))
                continue;

            int mod = 0;
            if (feniaModifier( ch, fam, paf->location, tier, mod )) {
                paf->level = level;
                paf->modifier = mod;
            }
        }
    }
    else {
        // First wear: add an affect for every location, exactly as the old
        // code did (Fenia answers 0 for the ones this family leaves flat, so
        // they are hung as zero affects just like before). A location is only
        // skipped if the whole Fenia module is unavailable.
        Affect af;
        af.type = -1;
        af.duration = -1;
        af.level = level;

        for (int i = 0; PERSONAL_APPLIES[i] != -1; i++) {
            int mod = 0;
            if (!feniaModifier( ch, fam, PERSONAL_APPLIES[i], tier, mod ))
                continue;

            af.location = PERSONAL_APPLIES[i];
            af.modifier = mod;
            affect_to_obj( obj, &af );
        }
    }
}

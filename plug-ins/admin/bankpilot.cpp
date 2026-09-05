/* bankpilot -- immortal round-trip proof for the object bank (phase 1).
 *
 * Deposits a carried item into a throwaway 'pilot' cell, reads it straight back,
 * and reports whether Id / permanent-affect / temp-affect-strip / subtree
 * fidelity survived the serialize -> extract -> materialize cycle.
 *
 * Throwaway dev harness: delete this command (and its XML) before the real
 * 'vault' ships. It is here only to de-risk the wrapper-Id-rebind assumption
 * on live before anything touches real hoards.
 */
#include <sstream>

#include "admincommand.h"
#include "save_bank.h"

#include "character.h"
#include "pcharacter.h"
#include "object.h"
#include "affect.h"
#include "loadsave.h"
#include "arg_utils.h"
#include "merc.h"
#include "def.h"

static int bp_count_subtree( Object *obj )
{
    int n = 1;
    for ( Object *c = obj->contains; c != 0; c = c->next_content )
        n += bp_count_subtree( c );
    return n;
}

static int bp_count_affects( Object *obj, bool permanent )
{
    int n = 0;
    for ( auto &paf : obj->affected ) {
        bool isPerm = ( paf->duration < 0 );
        if ( isPerm == permanent )
            n++;
    }
    return n;
}

CMDADM( bankpilot )
{
    if ( !ch->is_immortal( ) )
        return;

    DLString args = constArguments;
    DLString kw = args.getOneArgument( );

    if ( kw.empty( ) ) {
        ch->pecho( "Usage: bankpilot <item-in-inventory>" );
        return;
    }

    Object *obj = get_obj_carry( ch, kw );
    if ( obj == 0 ) {
        ch->pecho( "bankpilot: no '%s' in your inventory.", kw.c_str( ) );
        return;
    }

    // Capture BEFORE state: bank_deposit strips temp affects in place and then
    // frees obj via extract, so nothing below may dereference obj afterwards.
    long long beforeId   = obj->getID( );
    int       beforeVnum = obj->pIndexData->vnum;
    int       beforeSub  = bp_count_subtree( obj );
    int       beforePerm = bp_count_affects( obj, true );
    int       beforeTemp = bp_count_affects( obj, false );

    ch->pecho( "{WbankPILOT{x deposit  [%d] Id %lld: subtree %d, affects perm %d / temp %d",
               beforeVnum, beforeId, beforeSub, beforePerm, beforeTemp );

    DLString kind = "pilot";
    DLString key  = ch->getPC( )->getName( );  // unique ASCII login, not a declined form

    if ( !bank_deposit( obj, kind, key ) ) {
        ch->pecho( "{RbankPILOT FAIL{x: deposit returned false." );
        return;
    }
    obj = 0; // freed by extract inside bank_deposit

    bank_withdraw_all( ch, kind, key );

    // Locate the re-materialized instance by matching Id.
    Object *back = 0;
    for ( Object *o = ch->carrying; o != 0; o = o->next_content ) {
        if ( o->getID( ) == beforeId ) {
            back = o;
            break;
        }
    }

    if ( back == 0 ) {
        ch->pecho( "{RbankPILOT FAIL{x: object Id %lld not found after withdrawal.", beforeId );
        return;
    }

    int afterSub  = bp_count_subtree( back );
    int afterPerm = bp_count_affects( back, true );
    int afterTemp = bp_count_affects( back, false );

    ch->pecho( "{WbankPILOT{x recover  [%d] Id %lld: subtree %d, affects perm %d / temp %d",
               back->pIndexData->vnum, back->getID( ), afterSub, afterPerm, afterTemp );

    bool idOk   = ( back->getID( ) == beforeId );
    bool subOk  = ( afterSub == beforeSub );
    bool permOk = ( afterPerm == beforePerm );
    bool tempOk = ( afterTemp == 0 );        // every temporary affect must be gone

    ch->pecho( "  Id preserved ........ %s", idOk   ? "{GPASS{x" : "{RFAIL{x" );
    ch->pecho( "  subtree preserved ... %s (%d -> %d)", subOk  ? "{GPASS{x" : "{RFAIL{x", beforeSub, afterSub );
    ch->pecho( "  permanent affects ... %s (%d -> %d)", permOk ? "{GPASS{x" : "{RFAIL{x", beforePerm, afterPerm );
    ch->pecho( "  temp affects stripped %s (%d -> %d)", tempOk ? "{GPASS{x" : "{RFAIL{x", beforeTemp, afterTemp );
    ch->pecho( "{WbankPILOT{x overall: %s",
               ( idOk && subOk && permOk && tempOk ) ? "{GALL PASS{x" : "{RFAIL{x" );

    // Honesty about what this harness does NOT prove:
    ch->pecho( "{y  scope: Id / subtree / affect-count only. Fenia instance fields are" );
    ch->pecho( "{y  NOT checked -- probe live: eval a field onto the item, bankpilot it," );
    ch->pecho( "{y  then eval the field on the recovered object (same Id)." );
    ch->pecho( "{y  Test with a plain 'load obj' item: random/tiered weapons get affects" );
    ch->pecho( "{y  reassigned on load and will read as a false affect-count FAIL.{x" );
}

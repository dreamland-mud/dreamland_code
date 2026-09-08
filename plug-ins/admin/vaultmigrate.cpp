/* vaultmigrate -- one-time cleanup of owned "litter" containers into player vaults.
 *
 * The object bank replaces the old habit of leaving personal quest bags and
 * renamed stash chests parked on the ground (and in the Lost and Found), each of
 * which rides the obj_update sweep forever. This command finds every such owned
 * container and, in the 'go' phase, moves its CONTENTS into the owner's vault
 * cell and removes the empty container. Loose personal items in the bureau (quest
 * rings, girths, hero gear) are NOT containers and are left untouched, so the Lost
 * and Found keeps working for them.
 *
 * This file ships the DRY phase only: it classifies and reports, and writes
 * nothing. The destructive 'go' phase is a reviewed follow-up built against the
 * manifest this produces.
 */
#include <map>
#include <vector>
#include <sstream>
#include <fstream>

#include "logstream.h"
#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "core/object.h"
#include "room.h"
#include "clanreference.h"
#include "comm.h"
#include "save.h"
#include "save_bank.h"
#include "loadsave.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

// Lost-and-Found room vnums (loadsave/vnum.h): BUREAU_1 3078 historical,
// BUREAU_2 3083 active takeable, BUREAU_3 3084 old-chest storage. Hard-coded to
// avoid a cross-plugin include of the loadsave header; the values are stable.
#define VM_BUREAU_1 3078
#define VM_BUREAU_2 3083
#define VM_BUREAU_3 3084
#define VM_QUESTBAG  103   // the quest bag; the mansion-103 sweep targets only this vnum

CLAN(none);

/* Litter = an OWNED container sitting directly in a room, sparing mansion /
 * gods-only / clan-hall rooms and any object deliberately pinned with the
 * 'keepHere' property. The item_type==container test is what keeps loose personal
 * items (quest rings, girths, hero weapons) out of scope -- they are not
 * containers, so the bureau still works for them. Take flag is NOT a filter: a
 * hero quest bag in the Lost and Found is takeable and still counts. */
static bool vaultmigrate_is_litter( Object *obj )
{
    if (obj->pIndexData == 0)
        return false;
    if (obj->getOwner( ).empty( ))
        return false;
    if (obj->in_room == 0)                      // only what sits on the ground
        return false;
    if (obj->item_type != ITEM_CONTAINER)       // bags and chests only
        return false;

    Room *r = obj->in_room;
    if (IS_SET(r->room_flags, ROOM_MANSION|ROOM_GODS_ONLY))
        return false;
    if (r->pIndexData->clan != clan_none)       // spare clan halls
        return false;
    if (!obj->getProperty( "keepHere" ).empty( ))
        return false;

    return true;
}

// Like is_litter, but for the mansion-103 sweep: only quest bags (VM_QUESTBAG),
// and mansion rooms are NOT spared -- a dead player's bag parked in a live player's
// mansion, or a live owner's bag in their own mansion, still counts. Carried and
// nested bags are excluded (in_room != 0); gods-only / clan / keepHere stay spared.
// The per-bag owner test in the go-phase keeps a live owner's own non-103 property
// untouched -- only the owned quest bag is ever acted on.
static bool vaultmigrate_is_mansion103( Object *obj )
{
    if (obj->pIndexData == 0)
        return false;
    if (obj->getOwner( ).empty( ))
        return false;
    if (obj->in_room == 0)                       // parked on a floor only -- no carried/nested
        return false;
    if (obj->item_type != ITEM_CONTAINER)
        return false;
    if (obj->pIndexData->vnum != VM_QUESTBAG)    // ONLY the quest bag
        return false;

    Room *r = obj->in_room;
    if (IS_SET(r->room_flags, ROOM_GODS_ONLY))   // still spare gods-only (NOT mansion)
        return false;
    if (r->pIndexData->clan != clan_none)        // still spare clan halls
        return false;
    if (!obj->getProperty( "keepHere" ).empty( ))
        return false;

    return true;
}

// Whole subtree size (everything that would move to the vault / be freed).
static int vaultmigrate_count_contents( Object *obj )
{
    int n = 0;
    for (Object *c = obj->contains; c != 0; c = c->next_content) {
        n++;
        n += vaultmigrate_count_contents( c );
    }
    return n;
}

// How many nodes in the subtree the vault would REFUSE: limited (limit != -1) or
// carrying a running timer. Mirrors vault_policy_reason's per-node test (vault.cpp).
// A container with any refused node can't be fully emptied+killed under v1 policy.
static int vaultmigrate_count_refused( Object *obj )
{
    int n = 0;
    for (Object *c = obj->contains; c != 0; c = c->next_content) {
        if ( ( c->pIndexData != 0 && c->pIndexData->limit != -1 ) || c->timer > 0 )
            n++;
        n += vaultmigrate_count_refused( c );
    }
    return n;
}

// Which bucket a litter container's room falls into, for the report.
static const char * vaultmigrate_bucket( int vnum )
{
    if (vnum == VM_BUREAU_1) return "bureau-3078";
    if (vnum == VM_BUREAU_2) return "bureau-3083";
    if (vnum == VM_BUREAU_3) return "storage-3084";
    return "world";
}

struct VMOwnerTally {
    bool exists;
    int containers;
    int items;
    VMOwnerTally( ) : exists(false), containers(0), items(0) { }
};

/* Destructive migration (policy B): move each litter bag's contents into the
 * OWNER's vault, then delete the emptied bag. Limited and timered items migrate
 * too (they only pause -- the bank freezes the serialized timer and it resumes on
 * withdrawal); the sole carve-out is NOSAVEDROP, which bank_deposit refuses
 * because the serializer would silently destroy it, so those items (and their
 * bag) are left in place. Bags whose owner has no live profile are purged (no
 * vault to receive). Each affected room is re-saved immediately after its bag
 * changes, so a deletion cannot respawn on the next reboot from a stale room save
 * (which would also dupe against the vault copy). Targets one owner (pilot this
 * first) or `all`. */
static void vaultmigrate_go( Character *ch, DLString rest, bool mansion103 )
{
    DLString target = rest.getOneArgument( );
    if (target.empty( )) {
        ch->pecho( "Usage: vaultmigrate go <owner>   (one owner -- pilot this first)" );
        ch->pecho( "       vaultmigrate go all       (every litter bag -- destructive)" );
        return;
    }
    DLString targetKey = target.toLower( );
    bool doAll = (targetKey == "all");

    // Pass 1: snapshot the target bags. bank_deposit/extract_obj mutate
    // object_list, so we must not delete while walking it. Litter bags sit
    // directly in rooms and are never nested in one another, so these pointers
    // stay valid across pass 2.
    std::vector<Object *> bags;
    for (Object *obj = object_list; obj != 0; obj = obj->next) {
        if (!( mansion103 ? vaultmigrate_is_mansion103( obj ) : vaultmigrate_is_litter( obj ) ))
            continue;
        if (!doAll && obj->getOwner( ).toLower( ) != targetKey)
            continue;
        bags.push_back( obj );
    }

    if (bags.empty( )) {
        if (doAll)
            ch->pecho( "No litter bags found -- nothing to migrate." );
        else
            ch->pecho( "No litter bag owned by '" + target + "'." );
        return;
    }

    int bagsEmptied = 0, bagsPurged = 0, bagsLeft = 0;
    int itemsBanked = 0, itemsPurged = 0, itemsLeft = 0;

    for (size_t i = 0; i < bags.size( ); i++) {
        Object *bag = bags[i];
        Room *room = bag->in_room;                  // captured before any extract
        DLString owner = bag->getOwner( );
        bool exists = PCharacterManager::find( owner ) != 0;

        // Suppress the engine's per-deposit auto-save of the WHOLE room. bank_deposit
        // -> extract_obj_nocount -> obj_from_obj -> save_items_at_holder re-serializes
        // every object still in the room on EVERY item removed; on bureau 3083 (36
        // bags, ~10k items) that is an O(n^2) main-loop freeze. reset_room brackets
        // its resets the same way. We save the room ONCE per bag, after restoring the
        // flag -- save_items no-ops while DL_SAVE_OBJS is off, so it must come after
        // resetOption. One save per bag keeps the crash window per-bag (the same
        // window `vault put all` already accepts).
        dreamland->removeOption( DL_SAVE_OBJS );

        if (!exists) {
            // No vault to receive -- purge the whole bag + subtree.
            itemsPurged += vaultmigrate_count_contents( bag );
            extract_obj( bag );                     // recurses into contents
            bagsPurged++;
        } else {
            // Deposit each top-level content item into the owner's vault. Capture the
            // next pointer BEFORE bank_deposit -- a true return extracts the item, so
            // reading it->next_content afterwards would be a use-after-free.
            DLString key = owner.toLower( );
            Object *next = 0;
            for (Object *it = bag->contains; it != 0; it = next) {
                next = it->next_content;
                if (bank_deposit( it, "player", key ))
                    itemsBanked++;
                else
                    itemsLeft++;                    // NOSAVEDROP / write failure -- stays in the bag
            }

            if (bag->contains == 0) {
                extract_obj( bag );                 // fully emptied -> remove the litter bag
                bagsEmptied++;
            } else {
                bagsLeft++;                         // NOSAVEDROP leftovers -> keep the bag
            }
        }

        dreamland->resetOption( DL_SAVE_OBJS );

        // Persist the room now (bag deleted, or its contents changed) so a reboot
        // loading the stale room save can't undo this or dupe the banked items.
        if (room != 0)
            save_items( room );
    }

    std::ostringstream buf;
    buf << "{WVault migration -- GO";
    if (!doAll)
        buf << " (" << target << ")";
    buf << "{x  --  changes are LIVE and saved.\n\n";
    buf << "Bags emptied + deleted   : " << bagsEmptied << "\n";
    buf << "Items banked to vaults   : " << itemsBanked << "\n";
    if (bagsPurged > 0)
        buf << "Deleted-owner bags purged: " << bagsPurged
            << "   (" << itemsPurged << " items, no vault to receive)\n";
    if (bagsLeft > 0)
        buf << "{YBags KEPT{x                 : " << bagsLeft
            << "   (" << itemsLeft << " items couldn't bank -- NOSAVEDROP, or a write error)\n";

    page_to_char( buf.str( ).c_str( ), ch );
}

CMDADM( vaultmigrate )
{
    if (!ch->isCoder( )) {
        ch->pecho( "vaultmigrate: coders only." );
        return;
    }

    DLString rest = constArguments;
    DLString arg = rest.getOneArgument( );
    bool mansion103 = (arg == "go103" || arg == "dry103");

    if (arg == "go" || arg == "go103") {
        vaultmigrate_go( ch, rest, mansion103 );
        return;
    }

    if (arg != "dry" && arg != "dry103") {
        ch->pecho( "Usage:" );
        ch->pecho( "  vaultmigrate dry             report only -- writes nothing" );
        ch->pecho( "  vaultmigrate go <owner>      migrate one owner's litter bag(s) -- pilot this" );
        ch->pecho( "  vaultmigrate go all          migrate EVERY litter bag (destructive, one-time)" );
        ch->pecho( "  vaultmigrate dry103          like dry, but quest bags (vnum 103) IN mansions too" );
        ch->pecho( "  vaultmigrate go103 <owner>|all   migrate/purge those mansion quest bags" );
        return;
    }

    std::map<DLString, VMOwnerTally> byOwner;
    int totalContainers = 0, totalItems = 0;
    int bureauContainers = 0, worldContainers = 0;
    int deletedOwnerContainers = 0, deletedOwnerItems = 0;
    int totalRefused = 0, affectedContainers = 0;

    std::ostringstream dump;
    dump << "owner\texists\troom\tbucket\ttakeable\tvnum\tid\titems\trefused\n";

    for (Object *obj = object_list; obj != 0; obj = obj->next) {
        if (!( mansion103 ? vaultmigrate_is_mansion103( obj ) : vaultmigrate_is_litter( obj ) ))
            continue;

        DLString owner = obj->getOwner( );
        bool exists = PCharacterManager::find( owner ) != 0;
        int items = vaultmigrate_count_contents( obj );
        int refused = vaultmigrate_count_refused( obj );
        int vnum = obj->in_room->vnum;
        const char *bucket = vaultmigrate_bucket( vnum );
        bool takeable = obj->can_wear( ITEM_TAKE );

        VMOwnerTally &t = byOwner[owner];
        t.exists = exists;
        t.containers++;
        t.items += items;

        totalContainers++;
        totalItems += items;
        totalRefused += refused;
        if (refused > 0)
            affectedContainers++;
        if (vnum == VM_BUREAU_1 || vnum == VM_BUREAU_2 || vnum == VM_BUREAU_3)
            bureauContainers++;
        else
            worldContainers++;
        if (!exists) {
            deletedOwnerContainers++;
            deletedOwnerItems += items;
        }

        dump << owner << "\t" << (exists ? "yes" : "NO") << "\t"
             << obj->in_room->getName( ) << "\t" << bucket << "\t"
             << (takeable ? "take" : "notake") << "\t"
             << obj->pIndexData->vnum << "\t" << obj->getID( ) << "\t" << items << "\t" << refused << "\n";
    }

    // Full per-container manifest to a file for offline reading.
    const char *path = "vaultmigrate-dryrun.txt";
    std::ofstream fout( path );
    if (fout) {
        fout << dump.str( );
        fout.close( );
    }

    // Screen summary + per-owner table.
    std::ostringstream buf;
    buf << "{WVault migration -- DRY RUN (nothing was changed).{x\n\n";
    if (mansion103)
        buf << "Mansion-103 sweep = owned quest bags (vnum 103) parked in ANY room incl.\n"
               "mansions, sparing gods-only / clan / keepHere. Carried and nested bags excluded.\n\n";
    else {
        buf << "Litter = owned containers on the ground, sparing mansion/clan/gods-only rooms\n";
        buf << "and keepHere fixtures. Loose bureau items (rings/girths) are not containers and\n";
        buf << "are left alone.\n\n";
    }
    buf << "Containers found : " << totalContainers
        << "   (bureau " << bureauContainers << ", world " << worldContainers << ")\n";
    buf << "Items inside them: " << totalItems << "   (these would move to owner vaults)\n";
    buf << "Distinct owners  : " << (int)byOwner.size( ) << "\n";
    buf << "Deleted owners   : " << deletedOwnerContainers << " containers / "
        << deletedOwnerItems << " items would be PURGED (no vault to receive them)\n";
    buf << "Policy-refused   : " << totalRefused << " items (limited or timered) in "
        << affectedContainers << " containers -- the vault refuses these, so those bags\n";
    buf << "                   can't be fully emptied+killed under v1 policy.\n\n";
    buf << "{Wby owner{x (exists = has a live profile):\n";
    buf << "owner            exists  chests  items\n";

    for (std::map<DLString, VMOwnerTally>::iterator it = byOwner.begin( ); it != byOwner.end( ); ++it) {
        const DLString &name = it->first;
        buf << name;
        for (int i = (int)name.size( ); i < 17; i++)
            buf << " ";
        buf << (it->second.exists ? "yes   " : "{RNO {x   ")
            << "  " << it->second.containers
            << "\t" << it->second.items << "\n";
    }

    buf << "\nFull per-container manifest written to: " << path << "\n";

    page_to_char( buf.str( ).c_str( ), ch );
}

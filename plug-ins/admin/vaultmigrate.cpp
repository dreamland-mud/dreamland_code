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
#include "merc.h"
#include "def.h"

// Lost-and-Found room vnums (loadsave/vnum.h): BUREAU_1 3078 historical,
// BUREAU_2 3083 active takeable, BUREAU_3 3084 old-chest storage. Hard-coded to
// avoid a cross-plugin include of the loadsave header; the values are stable.
#define VM_BUREAU_1 3078
#define VM_BUREAU_2 3083
#define VM_BUREAU_3 3084

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

CMDADM( vaultmigrate )
{
    if (!ch->isCoder( )) {
        ch->pecho( "vaultmigrate: coders only." );
        return;
    }

    DLString rest = constArguments;
    DLString arg = rest.getOneArgument( );

    if (arg != "dry") {
        ch->pecho( "Usage: vaultmigrate dry   (report only -- writes nothing)" );
        ch->pecho( "The destructive 'go' phase is not built yet: this is the dry run." );
        return;
    }

    std::map<DLString, VMOwnerTally> byOwner;
    int totalContainers = 0, totalItems = 0;
    int bureauContainers = 0, worldContainers = 0;
    int deletedOwnerContainers = 0, deletedOwnerItems = 0;

    std::ostringstream dump;
    dump << "owner\texists\troom\tbucket\ttakeable\tvnum\tid\titems\n";

    for (Object *obj = object_list; obj != 0; obj = obj->next) {
        if (!vaultmigrate_is_litter( obj ))
            continue;

        DLString owner = obj->getOwner( );
        bool exists = PCharacterManager::find( owner ) != 0;
        int items = vaultmigrate_count_contents( obj );
        int vnum = obj->in_room->vnum;
        const char *bucket = vaultmigrate_bucket( vnum );
        bool takeable = obj->can_wear( ITEM_TAKE );

        VMOwnerTally &t = byOwner[owner];
        t.exists = exists;
        t.containers++;
        t.items += items;

        totalContainers++;
        totalItems += items;
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
             << obj->pIndexData->vnum << "\t" << obj->getID( ) << "\t" << items << "\n";
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
    buf << "Litter = owned containers on the ground, sparing mansion/clan/gods-only rooms\n";
    buf << "and keepHere fixtures. Loose bureau items (rings/girths) are not containers and\n";
    buf << "are left alone.\n\n";
    buf << "Containers found : " << totalContainers
        << "   (bureau " << bureauContainers << ", world " << worldContainers << ")\n";
    buf << "Items inside them: " << totalItems << "   (these would move to owner vaults)\n";
    buf << "Distinct owners  : " << (int)byOwner.size( ) << "\n";
    buf << "Deleted owners   : " << deletedOwnerContainers << " containers / "
        << deletedOwnerItems << " items would be PURGED (no vault to receive them)\n\n";
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

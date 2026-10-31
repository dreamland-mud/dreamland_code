/* recalc -- rebuild a player's armor[], hitroll and damroll from what is worn and the
 * active affects.
 *
 * Login already rebuilds them, so this is for an online character whose numbers
 * drifted, and for proving the recompute matches before armor_version is turned on.
 * A dry run by default: it prints the columns and writes nothing.
 */
#include <sstream>

#include "admincommand.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "tohit.h"
#include "merc.h"
#include "def.h"

CMDADM( recalc )
{
    DLString args = constArguments;
    DLString name = args.getOneArgument( );
    DLString mode = args.getOneArgument( );

    if (name.empty( ) || (!mode.empty( ) && mode != "apply")) {
        ch->pecho( "Usage: recalc <player>         show current and recomputed armor and rolls, write nothing" );
        ch->pecho( "       recalc <player> apply   write the recomputed armor and rolls" );
        return;
    }

    PCharacter *victim = PCharacterManager::findPlayer( name );
    if (!victim) {
        ch->pecho( "No such player online: login rebuilds armor by itself." );
        return;
    }

    int columns[4];
    if (!armor_recompute( victim, columns )) {
        ch->pecho( "The wearlocation plugin is not loaded." );
        return;
    }

    std::ostringstream buf;
    buf << victim->getName( ) << " armor version " << armor_version_of( victim )
        << " of " << to_hit_config( ).armor_version << "\r\n";
    for (int i = 0; i < 4; i++)
        buf << "  column " << i << ": " << victim->armor[i] << " -> " << columns[i]
            << (victim->armor[i] != columns[i] ? "  differs" : "") << "\r\n";

    int hitroll, damroll;
    roll_recompute( victim, hitroll, damroll );
    buf << "  hitroll: " << victim->hitroll.getValue( ) << " -> " << hitroll
        << (victim->hitroll != hitroll ? "  differs" : "") << "\r\n";
    buf << "  damroll: " << victim->damroll.getValue( ) << " -> " << damroll
        << (victim->damroll != damroll ? "  differs" : "") << "\r\n";
    ch->send_to( buf );

    if (mode != "apply") {
        ch->pecho( "Dry run, nothing written." );
        return;
    }

    for (int i = 0; i < 4; i++)
        victim->armor[i] = columns[i];

    victim->hitroll = hitroll;
    victim->damroll = damroll;

    if (to_hit_config( ).armor_version > 0)
        armor_version_stamp( victim, to_hit_config( ).armor_version );

    victim->save( );
    ch->pecho( "Written." );
}

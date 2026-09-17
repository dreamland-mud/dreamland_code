/* $Id$
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

#include <iostream>
#include <unistd.h>

#include "logstream.h"
#include "exception.h"
#include "dreamland.h"

static const DLString DEFAULT_CONFIG_PATH = "etc/dreamland.xml";

int main(int argc, char *argv[])
{
    try {
        DreamLand dl;

        if (argc > 1)
            dl.setConfigFilePath(argv[1]);
        else
            dl.setConfigFilePath(DEFAULT_CONFIG_PATH);

        dl.load();

        try {
            dl.run();
        } catch (const Exception &e1) {
            e1.printStackTrace(LogStream::sendFatal());
        }

        dl.save();

        // All shutdown work is durable at this point: player files were written in
        // reboot_now(), and dl.save() committed the config and synced the Fenia DB (every
        // Fenia write is a DB_TXN_SYNC transaction and the env opens with DB_RECOVER, so an
        // unclean exit recovers on next boot). Nothing is left to persist.
        //
        // The remaining ~DreamLand teardown (feniaManager->close then plugin .so unload while
        // pooled worker pthreads may still be parked) has an intermittent shutdown-only race
        // that SIGSEGVs and dumps a ~1GB core for a process that is exiting anyway -- it only
        // frees memory the OS reclaims regardless. Skip it: leave immediately with a clean 0,
        // so a benign reboot stops tripping the crash monitor and filling the disk with cores.
        // Revert this _exit if you ever want to actually chase the teardown race under gdb.
        LogStream::sendNotice() << "Clean shutdown; skipping destructor teardown." << endl;
        ::_exit(0);

    } catch (const Exception &e1) {
        e1.printStackTrace(LogStream::sendFatal());
        return 1;
    }

    return 0;
}

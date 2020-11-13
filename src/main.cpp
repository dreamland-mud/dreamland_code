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

    } catch (const Exception &e1) {
        e1.printStackTrace(LogStream::sendFatal());
        return 1;
    }

    return 0;
}

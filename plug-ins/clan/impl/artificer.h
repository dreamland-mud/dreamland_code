/*
 * Artificers [A]: the Artel hall guard. The clan itself lives in Fenia;
 * the guard keeps the stock ClanGuard behavior (push intruders, fight
 * enemies, notify the clan) and only needs its own XML-allocatable class.
 */

#ifndef ARTIFICER_H
#define ARTIFICER_H

#include "clanmobiles.h"

class ClanGuardArtificer: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardArtificer> Pointer;
};

#endif

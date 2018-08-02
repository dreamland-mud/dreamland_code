/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLANREFERENCE_H__
#define __CLANREFERENCE_H__

#include "globalreference.h"
#include "xmlglobalreference.h"

#include "clanmanager.h"
#include "clan.h"

#define CLAN( name ) static ClanReference clan_##name( #name )

GLOBALREF_DECL(Clan)
XMLGLOBALREF_DECL(Clan)

#endif

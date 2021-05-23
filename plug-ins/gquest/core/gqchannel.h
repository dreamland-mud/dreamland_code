/* $Id: gqchannel.h,v 1.1.2.1 2005/09/10 21:13:00 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef GQCHANNEL_H
#define GQCHANNEL_H

#include <sstream>

#include "dlstring.h"

struct Area;
class Character;
class PCharacter;
class GlobalQuest;
class GlobalQuestInfo;

class GQChannel {
public:

    static void zecho( GlobalQuest *, struct Area*, const DLString& );
    static void gecho( GlobalQuestInfo *, const DLString& );
    static void gecho( GlobalQuest *, const DLString&, PCharacter *pch = NULL);
    static void gecho( GlobalQuest *, ostringstream & );
    static void gecho( const DLString& );
    static void gecho( const DLString&, const DLString&, PCharacter *pch = NULL);
    static void pecho( Character *, const DLString& );
    static void pecho( Character *, ostringstream& );

    static const char * const BOLD;
    static const char * const NORMAL;
};

#endif

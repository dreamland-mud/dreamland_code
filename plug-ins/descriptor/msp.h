#ifndef MSP_H
#define MSP_H

#include "dlstring.h"

class Descriptor;

/** Whether this connection takes MSP sound triggers: a telnet client that
 *  answered DO MSP, or the web client, which always does. */
bool msp_enabled( Descriptor *d );

/**
 * Queue an MSP v0.3 !!SOUND line (https://www.zuggsoft.com/zmud/msp.htm) for
 * a sound from the web client's library, e.g. "alert/bell". The type is the
 * name's first path segment. volume 0-100, loops -1 (forever) or 1-100,
 * priority 0-100. False when the connection has no MSP or the name is bad.
 */
bool msp_sound( Descriptor *d, const DLString &name, int volume = 100, int loops = 1, int priority = 50 );

#endif

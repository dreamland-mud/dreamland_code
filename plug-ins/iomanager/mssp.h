#ifndef MSSP_H
#define MSSP_H

#include "dlstring.h"

class Descriptor;

/** Mud Server Status Protocol: answer MUD-list crawlers with name, player count, uptime and
 *  the static fields from config/mssp.json. Telnet (IAC SB MSSP) and plain-text forms. */
void mssp_send_telnet(Descriptor *d);
void mssp_send_plain(Descriptor *d);
bool mssp_is_request(const char *arg);

#endif

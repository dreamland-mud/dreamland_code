/* Saving a help article means saving the XML table element that owns it (a
 * skill, a religion, a help file...). XMLTableElement::save() drops the result,
 * so a failed write used to leave only a log line while the edit lived on in
 * memory until the next reboot. These helpers report the failure to Discord.
 */
#ifndef HELPSAVE_H
#define HELPSAVE_H

#include "dlstring.h"

class HelpArticle;
class XMLTableElement;

/** Save the owner of help through its table loader; false (and a Discord alert) on failure. */
bool help_save_owner(const HelpArticle *help, const XMLTableElement *owner);

/** Report that help was not written to disk; what names the owner. */
void help_save_failed(const HelpArticle *help, const DLString &what);

#endif

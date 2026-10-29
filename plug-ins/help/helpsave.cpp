#include "helpsave.h"
#include "helpmanager.h"
#include "xmltableelement.h"
#include "xmltableloader.h"
#include "messengers.h"
#include "logstream.h"

bool help_save_owner(const HelpArticle *help, const XMLTableElement *owner)
{
    XMLTableLoader *loader = owner ? owner->getLoader() : 0;

    if (loader && loader->saveElement(owner))
        return true;

    help_save_failed(help, owner ? owner->getName() : DLString("no owner"));
    return false;
}

void help_save_failed(const HelpArticle *help, const DLString &what)
{
    DLString msg = "Help " + DLString(help->getID()) + " (" + what
                   + ") was not written to disk: the edit lasts only until reboot.";

    LogStream::sendError() << msg << endl;
    send_discord_fenia("Help save failed", msg);
}

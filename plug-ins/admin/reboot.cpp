#include "admincommand.h"
#include "logstream.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "arg_utils.h"
#include "act.h"
#include "dreamland.h"

static void reboot_now( )
{
    Descriptor *d,*d_next;
    
    LogStream::sendNotice( ) << "Rebooting DREAM LAND." << endl;

    for ( d = descriptor_list; d != 0; d = d_next )
    {
            d_next = d->next;
            d->send("Мир Мечты уходит на перезагрузку ПРЯМО СЕЙЧАС!\n");

            if (d->character && d->connected == CON_PLAYING)
                    d->character->getPC( )->save();

            d->close();
    }
    dreamland->shutdown( );
}

void reboot_action(const DLString& constArguments, ostringstream& buf)
{
    DLString args = constArguments;
    DLString arg = args.getOneArgument();

    if (arg.empty())
    {
        buf << "Usage: reboot now" << endl;
        buf << "Usage: reboot <ticks to reboot>" << endl;
        buf << "Usage: reboot cancel" << endl;
        buf << "Usage: reboot status" << endl;
        return;
    }

    if (arg_is(arg, "cancel")) {
        dreamland->setRebootCounter(-1);
        buf << "Reboot canceled." << endl;
        return;
    }

    if (arg_is(arg, "now"))
    {
        buf << "Rebooting DreamLand NOW!" << endl;
        reboot_now();
        return;
    }

    if (arg_is(arg, "status"))
    {
        if (dreamland->getRebootCounter() == -1)
            buf << "Automatic rebooting is inactive." << endl;
        else
            buf << fmt(0, "Reboot in %d minutes.", dreamland->getRebootCounter()) << endl;
        return;
    }

    if (arg.isNumber())
    {
        dreamland->setRebootCounter(arg.toInt());
        buf << fmt(0, "Мир Мечты будет ПЕРЕЗАГРУЖЕН через %d тиков!", dreamland->getRebootCounter()) << endl;
        return;
    }

    reboot_action(DLString::emptyString, buf);
}

CMDADM( reboot )
{
    ostringstream buf;

    reboot_action(constArguments, buf);
    ch->send_to(buf);
}


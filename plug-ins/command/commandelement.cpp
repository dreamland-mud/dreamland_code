#include "logstream.h"
#include "commandelement.h"
#include "commandmanager.h"
#include "character.h"
#include "dreamland.h"

void CommandElement::loaded( )
{
    LogStream::sendNotice() << "Loaded command '" << Command::getName() << "'" << endl;
    commandManager->registrate(Pointer(this));
    linkWrapper();
}

void CommandElement::unloaded( )
{
    unlinkWrapper();
    commandManager->unregistrate(Pointer(this));
}

void CommandElement::run(Character *ch, const DLString &)
{
    ch->pecho("This command is under construction");
    LogStream::sendError() << "Legacy run called for CommandElement " << getName() << endl;
}

const DLString & CommandElement::getName() const
{
    return Command::getName();
}

void CommandElement::setName(const DLString&value)
{
    name[EN] = value;
}

bool CommandElement::saveCommand() const
{
    return getLoader()->saveElement(Pointer(this));
}


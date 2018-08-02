/* $Id$
 *
 * ruffina, 2004
 */
#include "inputhandler.h"
#include "descriptor.h"

template class XMLStub<InputHandler>;

/*-------------------------------------------------------------------
 * InputHandler
 *-------------------------------------------------------------------*/
int
InputHandler::handle(Descriptor *d, char *arg)
{
    d->send("No input handler!\n\r");
    /* assume this is 'ok' */
    return 0;
}

void
InputHandler::prompt(Descriptor *d)
{
    d->send("> ");
}

void InputHandler::close( Descriptor * )
{
}



/*---------------------------------------------------------------------
 * InputHandlerPlugin
 *---------------------------------------------------------------------*/
void InputHandlerPlugin::initialization( )
{
    Descriptor *d;
    const DLString &type = getType( );
    handle_input_t::iterator i;
    
    for(d = descriptor_list; d; d = d->next) 
	for(i = d->handle_input.begin(); i != d->handle_input.end(); i++)
	    if(*i && (*i)->getType() == type)
		i->recover();
}

void InputHandlerPlugin::destruction( )
{
    Descriptor *d;
    const DLString &type = getType( );
    handle_input_t::iterator i;
    
    for(d = descriptor_list; d; d = d->next) 
	for(i = d->handle_input.begin(); i != d->handle_input.end(); i++)
	    if(*i && (*i)->getType() == type)
		i->backup();
}


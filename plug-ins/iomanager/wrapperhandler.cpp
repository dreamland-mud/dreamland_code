/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "wrapperhandler.h"
#include "nannyhandler.h"
#include "descriptor.h"
#include "ban.h"
#include "comm.h"

#include "mercdb.h"
#include "def.h"

char * realip( char *ip );

void WrapperHandler::init( Descriptor *d ) 
{
    d->send("Waiting for hostname\n\r");
    d->handle_input.clear( );
    d->handle_input.push_front( new WrapperHandler );
}

int
WrapperHandler::handle(Descriptor *d, char *arg) 
{
    char *ip, *host, *key;

    key  = strtok(arg," "); 
    ip   = strtok(NULL," ");
    host = strtok(NULL," ");
    
    if (!key || strcmp(key,"amaltea")) {
	LogStream::sendWarning() << "Wrong keyword from " << d->realip << ": " << key << endl;
	d->send("Wrong keyword!\n\r");
	d->close( );
	return -1;
    }
    
    if (d->realip)
	free_string(d->realip);
    
    d->realip = realip( ip );
    
    if (d->host)
	free_string(d->host);
    
    d->host = realip( ip  );

    d->send("Got wrapper response.\n\r");
    
    if (banManager->checkVerbose( d, BAN_ALL )) {
	d->close( );
	return -1;
    }
    
    NannyHandler::init( d );
    return 1;
}

void
WrapperHandler::prompt(Descriptor *d)
{
}



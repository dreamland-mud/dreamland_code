/* $Id$
 *
 * ruffina, 2004
 */
#include "servicetrader.h"

#include "pcharacter.h"

/*----------------------------------------------------------------------
 * ServiceTrader
 *---------------------------------------------------------------------*/
void ServiceTrader::toStream( Character *client, ostringstream &buf )
{
    ServiceList::const_iterator s;

    for (s = services.begin( ); s != services.end( ); s++)
        if ((*s)->visible( client ))
            (*s)->toStream( client, buf );

}

Article::Pointer 
ServiceTrader::findArticle( Character *client, DLString &arg )
{
    Article::Pointer null;
    ServiceList::iterator s;
    
    if (arg.empty( ))
        return null;

    for (s = services.begin( ); s != services.end( ); s++) 
        if ((*s)->matches( arg )) 
            return *s;
    
    return null;
}   


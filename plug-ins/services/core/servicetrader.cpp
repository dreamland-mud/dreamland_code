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
    int number, n;
    
    number = arg.getNumberArgument( );    

    if (arg.empty( ))
        return null;

    for (n = 0, s = services.begin( ); s != services.end( ); s++) 
        if ((*s)->matches( arg )) 
            if (++n >= number) 
                return *s;
    
    return null;
}   


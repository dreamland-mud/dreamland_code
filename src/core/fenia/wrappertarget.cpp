/* $Id: wrappertarget.cpp,v 1.1.2.3.6.1 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "fenia/register-impl.h"

#include "wrapperbase.h"
#include "feniamanager.h"
#include "wrappertarget.h"

WrapperTarget::WrapperTarget( ) : wrapper( 0 )
{
}

WrapperBase * WrapperTarget::getWrapper( )
{
    if (wrapper && wrapper->hasHandler( ))
        return wrapper->getHandler( ).getDynamicPointer<WrapperBase>( );
    else
        return NULL;
}

void WrapperTarget::extractWrapper(bool forever)
{
    if (wrapper)
	getWrapper()->extract(forever);
}


/* $Id: wrappermanagerbase.cpp,v 1.1.4.1.6.1 2009/11/04 03:24:31 rufina Exp $
 *
 * ruffina, 2009
 */
#include "logstream.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "subr.h"

WrapperManagerBase::WrapperMap WrapperManagerBase::map;

void WrapperManagerBase::markAlive(long long id)
{
    WrapperMap::iterator i = map.find(id);

    if (i == map.end())
        return;

    WrapperBase *wbase = i->second->getHandler().getDynamicPointer<WrapperBase>();
    wbase->setAlive();
}


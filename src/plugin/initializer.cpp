/* $Id: initializer.cpp,v 1.1.2.3 2009/11/02 13:48:11 rufina Exp $
 * 
 * ruffina, Dream Land, 2008
 */
#include <stdlib.h>
#include "initializer.h"
#include "sharedobject.h"

Initializer::Initializer(int prio = INITPRIO_NORMAL)
{
    if(!SharedObject::current)
        abort();

    priority = prio;

    SharedObject::current->addInit(this);
}

Initializer::~Initializer()
{
    if(!SharedObject::current)
        abort();

    SharedObject::current->delInit(this);
}



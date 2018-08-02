/* $Id: oneallocate.h,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * ruffina, DreamLand, 2003
 */
/***************************************************************************
                          oneallocate.h  -  description
                             -------------------
    begin                : Mon May 14 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef ONEALLOCATE_H
#define ONEALLOCATE_H

#include "dlobject.h"

/**
 * @author Igor S. Petrenko
 */
class OneAllocate : public virtual DLObject
{
public:
    virtual ~OneAllocate( );
    
    static void checkDuplicate( DLObject* );
};

#endif

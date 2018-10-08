/* $Id: allocateclass.h,v 1.1.2.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * ruffina, DreamLand, 2003
 */
/***************************************************************************
                          allocateclass.h  -  description
                             -------------------
    begin                : Wed Oct 31 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef ALLOCATECLASS_H
#define ALLOCATECLASS_H

#include "dlobject.h"

class AllocateClass : public virtual DLObject
{
public:
    typedef ::Pointer<AllocateClass> Pointer;
    
    virtual ~AllocateClass( );

public:
    virtual DLObject::Pointer set( DLObject::Pointer arg1, DLObject::Pointer arg2 ) = 0;
};

#endif

/* $Id: skilleventhandler.h,v 1.1.2.1 2010-09-05 13:57:11 rufina Exp $
 * 
 * ruffina, 2010
 */
#ifndef __SKILLEVENTHANDLER_H__
#define __SKILLEVENTHANDLER_H__

#include <stdarg.h>

#include "xmlpolymorphvariable.h"
#include "skillaction.h"

class PCharacter;

class SkillEventHandler : public virtual SkillAction, public virtual XMLPolymorphVariable 
{
public:
    typedef ::Pointer<SkillEventHandler> Pointer;
    typedef bool (SkillEventHandler::*Method)( va_list args );
    
    virtual ~SkillEventHandler( );

    virtual bool putItem( va_list ) { return false; }
    virtual bool useItem( va_list ) { return false; }
    virtual bool dropItem( va_list ) { return false; }
    virtual bool fetchItem( va_list ) { return false; }
};

#endif

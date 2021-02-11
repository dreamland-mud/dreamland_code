/* $Id: affecthandler.h,v 1.1.2.3.18.5 2010-08-24 20:12:33 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __AFFECTHANDLER_H__
#define __AFFECTHANDLER_H__

#include <sstream>

#include "xmlpolymorphvariable.h"
#include "skillaction.h"
#include "wrappertarget.h"

class Affect;
class Character;
class Object;
class Room;

class AffectHandler : public virtual SkillAction, public WrapperTarget, public virtual XMLPolymorphVariable 
{
public:
    typedef ::Pointer<AffectHandler> Pointer;
    
    virtual ~AffectHandler( );

    virtual void remove( Character * ); 
    virtual void remove( Object * ); 
    virtual void remove( Room * ); 
    virtual void update( Character *, Affect * ); 
    virtual void update( Object *, Affect * ); 
    virtual void update( Room *, Affect * ); 
    virtual void entry( Character *, Affect * );
    virtual void entry( Room *, Character *, Affect * );
    virtual void leave( Room *, Character *, Affect * );
    virtual void dispel( Character * );
    virtual void look( Character *, Character *, Affect * );
    virtual bool smell( Character *, Character *, Affect * );
    virtual void toStream( ostringstream &, Affect * );
    virtual void saves( Character *, Character *, int &, int, Affect * );
    virtual void stopfol( Character *, Affect * );

    virtual bool isDispelled( ) const;
    virtual bool isCancelled( ) const;
};

#endif

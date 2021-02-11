/* $Id: defaultaffecthandler.h,v 1.1.2.6.10.7 2008/07/04 12:05:08 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __DEFAULTAFFECTHANDLER_H__
#define __DEFAULTAFFECTHANDLER_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "affecthandler.h"

class DefaultAffectHandler : public AffectHandler, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<DefaultAffectHandler> Pointer;
    
    DefaultAffectHandler( );

    virtual long long getID() const;

    virtual void setSkill( SkillPointer );
    virtual void unsetSkill( );
    virtual SkillPointer getSkill( ) const;

    virtual void remove( Character * ); 
    virtual void remove( Object * ); 
    virtual void remove( Room * ); 
    virtual void dispel( Character * );
    
    virtual bool isDispelled( ) const;
    virtual bool isCancelled( ) const;

protected:
    XML_VARIABLE XMLStringNoEmpty  wearoff;
    XML_VARIABLE XMLStringNoEmpty  wearoffObj;
    XML_VARIABLE XMLStringNoEmpty  wearoffRoom;
    XML_VARIABLE XMLStringNoEmpty  wearoffDispel;
    XML_VARIABLE XMLBooleanNoFalse dispelled, cancelled;
    
    SkillPointer skill;
};

#endif

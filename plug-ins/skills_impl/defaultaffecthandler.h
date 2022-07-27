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

    XML_VARIABLE XMLStringNoEmpty  removeCharSelf;
    XML_VARIABLE XMLStringNoEmpty  removeCharOthers;
    XML_VARIABLE XMLStringNoEmpty  removeObj;
    XML_VARIABLE XMLStringNoEmpty  removeRoom;
    XML_VARIABLE XMLBooleanNoFalse dispelled, cancelled;
    
    SkillPointer skill;
};

#endif

/* $Id$
 *
 * ruffina, 2004
 */
#ifndef OLCSTUBSKILL_H
#define OLCSTUBSKILL_H

#include "xmlenumeration.h"
#include "skillgroup.h"
#include "basicskill.h"

class OLCStubSkill : public BasicSkill {
XML_OBJECT
public:
    typedef ::Pointer<OLCStubSkill> Pointer;
    
    OLCStubSkill( );
    virtual ~OLCStubSkill( );

    virtual SkillGroupReference & getGroup( );
    virtual const DLString & getCategory( ) const;
    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const; 
    virtual int getLevel( Character * ) const;
    virtual void show( PCharacter *, ostream & buf );

protected:
    static const DLString CATEGORY;                                             
    XML_VARIABLE XMLSkillGroupReference group;
};


#endif

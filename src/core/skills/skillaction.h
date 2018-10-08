/* $Id$
 *
 * ruffina, 2004
 */
#ifndef        __SKILLACTION_H__
#define __SKILLACTION_H__

#include "pointer.h"
#include "dlobject.h"

class Skill;
typedef ::Pointer<Skill> SkillPointer;

class SkillAction : public virtual DLObject {
public:
    typedef ::Pointer<SkillAction> Pointer;
    
    virtual ~SkillAction( );

    virtual SkillPointer getSkill( ) const = 0;
    virtual void setSkill( SkillPointer ) = 0;
    virtual void unsetSkill( ) = 0;
};

#endif

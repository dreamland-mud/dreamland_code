#ifndef DREAMSKILL_H
#define DREAMSKILL_H_

#include "oneallocate.h"
#include "schedulertaskroundplugin.h"
#include "schedulertaskroundpcharacter.h"

class Skill;
class Character;

class DreamSkillManager : public virtual SchedulerTaskRoundPCharacter,
                          public virtual SchedulerTaskRoundPlugin,
                          public OneAllocate
{
public:
    typedef ::Pointer<DreamSkillManager> Pointer;
    
    DreamSkillManager();
    virtual ~DreamSkillManager();
    
    virtual void run( PCharacter* );
    virtual void after( );

protected:
    void describeDream(PCharacter *ch, Skill *skill) const;
    Skill * findRandomProfSkill(PCharacter *ch) const;
    int findActiveDreamSkill(PCharacter *ch) const;
    long findLatestDreamSkill(PCharacter *ch) const;
};

extern DreamSkillManager *dreamSkillManager;

#endif

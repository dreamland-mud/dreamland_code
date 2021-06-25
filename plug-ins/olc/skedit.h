#ifndef __SKEDIT_H__
#define __SKEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "skillreference.h"
#include "skillgroup.h"

class BasicSkill;
class DefaultSpell;
class DefaultAffectHandler;
class DefaultSkillCommand;
class DefaultSkillGroup;

class OLCStateSkill : public OLCStateTemplate<OLCStateSkill>,
                         public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateSkill> Pointer;

    OLCStateSkill();
    OLCStateSkill(Skill *skill);
    virtual ~OLCStateSkill();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLSkillReference original;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    BasicSkill *getOriginal();
    DefaultSpell *getSpell(BasicSkill *skill = 0);
    DefaultAffectHandler *getAffect(BasicSkill *skill = 0);
    DefaultSkillCommand *getCommand(BasicSkill *skill = 0);
    bool checkSpell(DefaultSpell *spell);
    bool checkAffect(DefaultAffectHandler *ah);
    bool checkCommand(DefaultSkillCommand *cmd);
    bool commandUpdate(DefaultSkillCommand *cmd);

    XML_VARIABLE XMLBoolean isChanged;
};

#define SKEDIT(C, rname, help) OLC_CMD(OLCStateSkill, C, rname, help)

class OLCStateSkillGroup : public OLCStateTemplate<OLCStateSkillGroup>,
                         public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateSkillGroup> Pointer;

    OLCStateSkillGroup();
    OLCStateSkillGroup(SkillGroup *group);
    virtual ~OLCStateSkillGroup();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLSkillGroupReference original;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    DefaultSkillGroup *getOriginal();

    XML_VARIABLE XMLBoolean isChanged;
};

#define GREDIT(C, rname, help) OLC_CMD(OLCStateSkillGroup, C, rname, help)

#endif

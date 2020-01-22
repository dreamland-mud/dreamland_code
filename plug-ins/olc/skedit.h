#ifndef __SKEDIT_H__
#define __SKEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "skillreference.h"
#include "basicskill.h"

class OLCStateSkill : public OLCStateTemplate<OLCStateSkill>,
                         public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateSkill> Pointer;

    OLCStateSkill();
    OLCStateSkill(Skill *rel);
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

    XML_VARIABLE XMLBoolean isChanged;
};

#define SKEDIT(C, rname, help) OLC_CMD(OLCStateSkill, C, rname, help)

#endif

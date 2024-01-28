#ifndef QEDIT_H
#define QEDIT_H

#include "olcstate.h"
#include "xmlinteger.h"

class AreaQuest;

class OLCStateAreaQuest : public OLCStateTemplate<OLCStateAreaQuest>,
                        public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateAreaQuest> Pointer;

    OLCStateAreaQuest();
    OLCStateAreaQuest(AreaQuest *q);
    virtual ~OLCStateAreaQuest();

    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLInteger vnum;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );

    AreaQuest *getOriginal();
    bool parseQuestVnum(PCharacter *ch, const DLString &arg, Integer &vnum);
    bool parseStepNumber(PCharacter *ch, const DLString &arg, Integer &step);

    XML_VARIABLE XMLBoolean isChanged;
};

#define AQEDIT(C, cname, help) OLC_CMD(OLCStateAreaQuest, C, cname, help)



#endif
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
    virtual DLString getJournalKind( ) const;
    virtual int getJournalVnum( ) const;
    void show( PCharacter * );

    XML_VARIABLE XMLInteger vnum;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );

    AreaQuest *getOriginal();
    bool parseStepNumber(PCharacter *ch, const DLString &arg, Integer &step);
    
    XML_VARIABLE XMLBoolean isChanged;
};

/** Step pairing rule: the end of step s and the begin of step s+1 sit on the
 *  same target, and an onX end pairs with a postX begin. Bits of what breaks. */
enum {
    STEP_PAIR_OK = 0,
    STEP_PAIR_TARGET = 1,
    STEP_PAIR_TRIGGER = 2
};
int qedit_step_pairing(AreaQuest *q, int s);

#define AQEDIT(C, cname, help) OLC_CMD(OLCStateAreaQuest, C, cname, help)



#endif
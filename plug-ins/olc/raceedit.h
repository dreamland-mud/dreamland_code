#ifndef __RACEEDIT_H__
#define __RACEEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"

class DefaultRace;
class DefaultPCRace;

class OLCStateRace : public OLCStateTemplate<OLCStateRace>,
                         public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateRace> Pointer;

    OLCStateRace();
    OLCStateRace(DefaultRace *rel);
    virtual ~OLCStateRace();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLRaceReference original;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    DefaultRace *getOriginal();
    DefaultPCRace *getPC();
    bool checkPC(DefaultPCRace *pc);

    XML_VARIABLE XMLBoolean isChanged;
};

#define RACEEDIT(C, rname, help) OLC_CMD(OLCStateRace, C, rname, help)

#endif

#ifndef __RELEDIT_H__
#define __RELEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "defaultreligion.h"

class OLCStateReligion : public OLCStateTemplate<OLCStateReligion>,
                         public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateReligion> Pointer;

    OLCStateReligion();
    OLCStateReligion(Religion *rel);
    virtual ~OLCStateReligion();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLReligionReference original;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    DefaultReligion *getOriginal();

    XML_VARIABLE XMLBoolean isChanged;
};

#define RELEDIT(C, rname, help) OLC_CMD(OLCStateReligion, C, rname, help)

#endif

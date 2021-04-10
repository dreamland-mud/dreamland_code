#ifndef __SOCEDIT_H__
#define __SOCEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "skillreference.h"

class Social;

class OLCStateSocial : public OLCStateTemplate<OLCStateSocial>,
                         public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateSocial> Pointer;

    OLCStateSocial();
    OLCStateSocial(Social *s);
    virtual ~OLCStateSocial();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLString socialName;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    Social * getOriginal();

    XML_VARIABLE XMLBoolean isChanged;
};

#define SOCEDIT(C, rname, help) OLC_CMD(OLCStateSocial, C, rname, help)

#endif

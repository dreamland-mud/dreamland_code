#ifndef __FEDIT_H__
#define __FEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"

class Configurable;

class OLCStateFile : public OLCStateTemplate<OLCStateFile>,
                     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateFile> Pointer;

    OLCStateFile();
    OLCStateFile(::Pointer<Configurable> cfg);
    virtual ~OLCStateFile();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * ) const;

    XML_VARIABLE XMLString path, text;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    bool validate( PCharacter * ) const;

    XML_VARIABLE XMLBoolean isChanged;
};

#define FEDIT(C, rname, help) OLC_CMD(OLCStateFile, C, rname, help)

#endif

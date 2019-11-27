#ifndef __HEDIT_H__
#define __HEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "helpmanager.h"

class OLCStateHelp : public OLCStateTemplate<OLCStateHelp>,
                     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateHelp> Pointer;

    OLCStateHelp();
    OLCStateHelp(HelpArticle *h);
    virtual ~OLCStateHelp();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * ) const;

    XML_VARIABLE XMLInteger id, level;
    XML_VARIABLE XMLString keywords;
    XML_VARIABLE XMLString fullKeyword;
    XML_VARIABLE XMLString text;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    HelpArticle::Pointer getOriginal() const;

    XML_VARIABLE XMLBoolean isChanged;
};

#define HEDIT(C, rname, help) OLC_CMD(OLCStateHelp, C, rname, help)

#endif

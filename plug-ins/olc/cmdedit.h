#ifndef CMDEDIT_H
#define CMDEDIT_H

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "commandplugin.h"

class OLCStateCommand : public OLCStateTemplate<OLCStateCommand>,
                        public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateCommand> Pointer;

    OLCStateCommand();
    OLCStateCommand(CommandPlugin *cmd);
    virtual ~OLCStateCommand();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * );

    XML_VARIABLE XMLString cmdName;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );

    CommandPlugin *original;
    CommandPlugin *getOriginal();
    bool commandUpdate(CommandPlugin *);
    CommandHelp::Pointer resolveHelp(CommandPlugin *);

    XML_VARIABLE XMLBoolean isChanged;
};

#define CMDEDIT(C, cname, help) OLC_CMD(OLCStateCommand, C, cname, help)


#endif

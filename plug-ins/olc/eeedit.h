/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __EEEDIT_H__
#define __EEEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlmultistring.h"

class OLCStateExtraExit : public OLCStateTemplate<OLCStateExtraExit>,
                          public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateExtraExit> Pointer;
    
    OLCStateExtraExit( );
    OLCStateExtraExit( RoomIndexData *, const DLString &name );
    virtual ~OLCStateExtraExit( );

    virtual void commit( );
    virtual void changed( PCharacter * );
    void show(PCharacter *ch);

    XML_VARIABLE XMLInteger room, to_room, info, key;
    XML_VARIABLE XMLEnumeration max_size_pass;
    XML_VARIABLE XMLMultiString keyword, short_desc_from, short_desc_to;
    XML_VARIABLE XMLMultiString description, room_description;
    XML_VARIABLE XMLMultiString msgLeaveRoom, msgLeaveSelf;
    XML_VARIABLE XMLMultiString msgEntryRoom, msgEntrySelf;
    
    template <typename T>
    bool cmd( PCharacter *, char * );
private:
    virtual void statePrompt( Descriptor * );
};

#define EEEDIT(Cmd) OLC_CMD(OLCStateExtraExit, Cmd, "", "")

#endif

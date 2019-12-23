/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __EEEDIT_H__
#define __EEEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"

class OLCStateExtraExit : public OLCStateTemplate<OLCStateExtraExit>,
                          public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateExtraExit> Pointer;
    
    OLCStateExtraExit( );
    OLCStateExtraExit( Room *, const DLString &name );
    virtual ~OLCStateExtraExit( );

    virtual void commit( );
    virtual void changed( PCharacter * );
    void show(PCharacter *ch);

    XML_VARIABLE XMLInteger room, to_room, info, key;
    XML_VARIABLE XMLInteger moving_from, moving_mode_from, moving_to;
    XML_VARIABLE XMLInteger moving_mode_to, max_size_pass;
    XML_VARIABLE XMLString keyword, short_desc_from, short_desc_to;
    XML_VARIABLE XMLString description, room_description;
    
    template <typename T>
    bool cmd( PCharacter *, char * );
private:
    virtual void statePrompt( Descriptor * );
};

#define EEEDIT(Cmd) OLC_CMD(OLCStateExtraExit, Cmd, "", "")

#endif

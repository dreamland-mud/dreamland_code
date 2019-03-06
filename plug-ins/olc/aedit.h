/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __AEDIT_H__
#define __AEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"

class OLCStateArea : public OLCStateTemplate<OLCStateArea>,
                     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateArea> Pointer;

    OLCStateArea();
    OLCStateArea(AREA_DATA *o);
    virtual ~OLCStateArea();
    
    virtual void commit();
    virtual void changed( PCharacter * );

    bool checkOverlap(int lower, int upper);

    XML_VARIABLE XMLInteger vnum, security, age, nplayer;
    XML_VARIABLE XMLInteger low_range, high_range, min_vnum, max_vnum;
    XML_VARIABLE XMLFlags area_flag;
    XML_VARIABLE XMLString file_name, name, credits, resetmsg, authors, altname, translator, speedwalk;
    XML_VARIABLE XMLString behavior;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
};

#define AEDIT(C) OLC_CMD(OLCStateArea, C)

#endif

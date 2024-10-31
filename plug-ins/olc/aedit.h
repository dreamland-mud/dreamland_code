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
#include "xmlmultistring.h"

class OLCStateArea : public OLCStateTemplate<OLCStateArea>,
                     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateArea> Pointer;

    OLCStateArea();
    OLCStateArea(AreaIndexData *o);
    virtual ~OLCStateArea();
    
    virtual void commit();
    virtual void changed( PCharacter * );

    bool checkOverlap(int lower, int upper);

    XML_VARIABLE XMLInteger vnum, security;
    XML_VARIABLE XMLInteger low_range, high_range, min_vnum, max_vnum;
    XML_VARIABLE XMLFlags area_flag;
    XML_VARIABLE XMLString file_name, authors, translator;
    XML_VARIABLE XMLString behavior;
    XML_VARIABLE XMLMultiString name, altname, resetMessage, speedwalk;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
};

#define AEDIT(C, rname, help) OLC_CMD(OLCStateArea, C, rname, help)

#endif

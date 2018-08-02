/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __TEDIT_H__
#define __TEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"

class OLCStateTrap : public OLCStateTemplate<OLCStateTrap>,
		     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateTrap> Pointer;
    
    OLCStateTrap( );
    OLCStateTrap( Room *, int i );
    virtual ~OLCStateTrap( );

    virtual void commit( );
    virtual void changed( PCharacter * );

    XML_VARIABLE XMLInteger room, num;
    XML_VARIABLE XMLInteger target, surface_quality, diving_speed, optimal_move;
    XML_VARIABLE XMLFlags info;
    XML_VARIABLE XMLString short_desc;
    XML_VARIABLE XMLString trap_on_s, trap_on_o; 
    XML_VARIABLE XMLString diving_on_s, diving_on_o;
    XML_VARIABLE XMLString move_on_s, move_on_o;
    
    template <typename T>
    bool cmd( PCharacter *, char * );
private:
    virtual void statePrompt( Descriptor * );
};

#define TEDIT(Cmd) OLC_CMD(OLCStateTrap, Cmd)

#endif

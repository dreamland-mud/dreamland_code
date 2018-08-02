/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __SEDIT_H__
#define __SEDIT_H__

#include "xmlstring.h"
#include "xmlpcstringeditor.h"

class OLCState;

class OLCStringEditor : public XMLPCStringEditor
{
public:
    OLCStringEditor(OLCState &s);

    virtual Descriptor *getOwner( );
    virtual void done();

    OLCState &olc;
};

#endif

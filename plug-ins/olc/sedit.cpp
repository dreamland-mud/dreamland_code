/* $Id$
 *
 * ruffina, 2004
 */

#include "colour.h"
#include "sedit.h"
#include "interp.h"
#include "dlstring.h"
#include "pcharacter.h"
#include "mercdb.h"
#include "olcstate.h"

OLCStringEditor::OLCStringEditor(OLCState &s) : olc(s)
{
}

void
OLCStringEditor::done( )
{
    olc.seditDone( );
}

Descriptor *
OLCStringEditor::getOwner( )
{
    /*XXX - throw somth?*/
    return olc.owner;
}


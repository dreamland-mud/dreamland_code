/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OEDIT_H__
#define __OEDIT_H__

#include "olcstate.h"
#include "xmlindexdata.h"

class OLCStateObject :  public OLCStateTemplate<OLCStateObject>, 
                        public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateObject> Pointer;

    OLCStateObject( );                        /*moc constructor*/
    OLCStateObject( OBJ_INDEX_DATA * );        /*edit original mobile*/
    OLCStateObject( int );                /*create new*/
    virtual ~OLCStateObject( );

    virtual void commit( );
    virtual void changed( PCharacter * );

    bool oedit_values(Character * ch, char *argument, int value);

    XML_VARIABLE XMLObjIndexData obj;

    template <typename T>
    bool cmd( PCharacter *, char * );
private:
    virtual void statePrompt( Descriptor * );

    void copyParameters( OBJ_INDEX_DATA * );
    void copyDescriptions( OBJ_INDEX_DATA * );
};

#define OEDIT(Cmd) OLC_CMD(OLCStateObject, Cmd, "", "")

#endif

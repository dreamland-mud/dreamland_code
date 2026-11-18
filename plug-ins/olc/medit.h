/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __MEDIT_H__
#define __MEDIT_H__

#include "olcstate.h"
#include "xmlindexdata.h"

class OLCStateMobile :  public OLCStateTemplate<OLCStateMobile>, 
                        public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateMobile> Pointer;

    OLCStateMobile( );                        /*moc constructor*/
    OLCStateMobile( MOB_INDEX_DATA * );        /*edit original mobile*/
    OLCStateMobile( int );                /*create new*/
    virtual ~OLCStateMobile( );

    virtual void commit( );
    virtual void changed( PCharacter * );

    /** 'delete <vnum>': flag the prototype for removal on the next area save.
     *  Returns it, or 0 with the reason told to ch. */
    static MOB_INDEX_DATA *markDeleted( PCharacter *, const char * );
    virtual DLString getJournalKind( ) const;
    virtual int getJournalVnum( ) const;

    XML_VARIABLE XMLMobIndexData mob;

    template <typename T>
    bool cmd( PCharacter *, char * );
private:
    virtual void statePrompt( Descriptor * );

    void copyParameters( MOB_INDEX_DATA * );
    void copyDescriptions( MOB_INDEX_DATA * );
    void copyBehaviors( MOB_INDEX_DATA * );
};

#define MEDIT(Cmd) OLC_CMD(OLCStateMobile, Cmd, "", "")

#endif

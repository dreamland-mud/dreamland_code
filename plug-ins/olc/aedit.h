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
#include <set>

class AreaQuest;

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

    virtual DLString getJournalKind( ) const;
    virtual int getJournalVnum( ) const;

    bool checkOverlap(int lower, int upper);
    /** Why committing this state would lose data on the next save: the vnum
     *  range leaves out a live prototype of the area, or another area already
     *  saves into file_name. Empty when it is safe. */
    DLString commitRefusal() const;

    /** 'quest create [<vnum>]': a new empty quest in the area, the first free
     *  vnum of its range when argVnum is empty. 0 with the reason told to ch. */
    /** First vnum of the area's range no quest uses and skip doesn't hold, or -1. */
    static int nextQuestVnum(AreaIndexData *original, const std::set<int> *skip = 0);
    static AreaQuest *questCreate(PCharacter *ch, AreaIndexData *original, const DLString &argVnum);
    /** 'quest delete <vnum>'. The quest's Fenia handlers stay: 'cs search <vnum>'. */
    static bool questDelete(PCharacter *ch, AreaIndexData *original, const DLString &arg);

    XML_VARIABLE XMLInteger vnum, security;
    XML_VARIABLE XMLInteger low_range, high_range, min_vnum, max_vnum;
    XML_VARIABLE XMLFlags area_flag;
    XML_VARIABLE XMLString file_name, authors, translator;
    XML_VARIABLE XMLString behavior;
    XML_VARIABLE XMLMultiString name, altname, resetMessage, speedwalk, preposition;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
};

#define AEDIT(C, rname, help) OLC_CMD(OLCStateArea, C, rname, help)

#endif

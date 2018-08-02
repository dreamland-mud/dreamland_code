/* $Id: mobiles.h,v 1.1.2.6.6.2 2008/03/06 17:48:35 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef LOCATEQUEST_MOBILES_H
#define LOCATEQUEST_MOBILES_H

#include "mobquestbehavior.h"

class LocateQuest;

class LocateCustomer : public TalkativeClient, 
                       public MandatoryMobile,
		       public ProtectedClient,
		       public PeacefulClient,
		       public GreedyClient,
		       public DedicatedMobile<LocateQuest>
{
XML_OBJECT
public:
    typedef ::Pointer<LocateCustomer> Pointer;

    virtual void show( Character *, std::basic_ostringstream<char> & );
    
protected:
    virtual void talkToHero( PCharacter * );
    virtual void deadFromIdiot( PCMemoryInterface * );
    virtual void deadFromSuicide( PCMemoryInterface * );
    virtual void deadFromKill( PCMemoryInterface *, Character * );
    virtual bool givenCheck( PCharacter *, Object * );
    virtual void givenGood( PCharacter *, Object * );
    virtual void givenBad( PCharacter *, Object * );
};


#endif


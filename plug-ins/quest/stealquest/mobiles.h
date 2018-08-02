/* $Id: mobiles.h,v 1.1.2.5.6.2 2008/03/06 17:48:36 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef STEALQUEST_MOBILES_H
#define STEALQUEST_MOBILES_H

#include "mobquestbehavior.h"

class StealQuest;

class RobbedVictim : public TalkativeClient, 
                     public MandatoryMobile,
		     public ProtectedClient,
		     public GreedyClient,
		     public DedicatedMobile<StealQuest>
{
XML_OBJECT
public:
    typedef ::Pointer<RobbedVictim> Pointer;

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

class Robber : public MobQuestBehavior {
XML_OBJECT
public:
    typedef ::Pointer<Robber> Pointer;
    
    virtual void show( Character *, std::basic_ostringstream<char> & );
};

#endif

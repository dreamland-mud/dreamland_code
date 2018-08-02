/* $Id: steakcustomer.h,v 1.1.2.6.22.2 2008/03/06 17:48:29 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef STEAKCUSTOMER_H
#define STEAKCUSTOMER_H

#include "mobquestbehavior.h"

class ButcherQuest;

class SteakCustomer : public MandatoryMobile, 
                      public ProtectedClient,
		      public GreedyClient,
		      public DedicatedMobile<ButcherQuest>
{
XML_OBJECT
public:
    typedef ::Pointer<SteakCustomer> Pointer;

    virtual void greet( Character *victim );

protected:
    virtual void deadAction( QuestPointer, PCMemoryInterface *, Character * );
    virtual bool givenCheck( PCharacter *, Object * );
    virtual void givenGood( PCharacter *, Object * );
    virtual void givenBad( PCharacter *, Object * );
};

#endif


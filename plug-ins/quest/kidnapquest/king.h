/* $Id: king.h,v 1.1.2.7.6.2 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef KIDNAPKING_H
#define KIDNAPKING_H

#include "kidnapmobile.h"

class KidnapKing : public KidnapMobile, 
                   public TalkativeClient, 
		   public MandatoryMobile,
		   public ProtectedClient,
		   public PeacefulClient
{
XML_OBJECT
public:
    typedef ::Pointer<KidnapKing> Pointer;

protected:
    virtual void talkToHero( PCharacter * );
    virtual void deadAction( QuestPointer, PCMemoryInterface *, Character * );

private:
    Object * giveMarkHero( PCharacter * );
};

#endif


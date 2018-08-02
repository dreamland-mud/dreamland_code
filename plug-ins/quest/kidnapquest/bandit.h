/* $Id: bandit.h,v 1.1.2.9.6.4 2009/09/24 14:09:13 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KIDNAPBANDIT_H
#define KIDNAPBANDIT_H

#include "kidnapmobile.h"

#include "schedulertask.h"

#include "wanderer.h"

enum {
    BSTAT_HUNT_PRINCE,
    BSTAT_KIDNAP,
    BSTAT_FIGHT,
    BSTAT_SLEEP,
};

class KidnapBandit : public KidnapMobile, 
                     public Wanderer, 
		     public ConfiguredMobile 
{
XML_OBJECT
public:
	typedef ::Pointer<KidnapBandit> Pointer;
    
	KidnapBandit( );

	virtual void fight( Character *victim );
	virtual void greet( Character *victim );
	virtual bool extractNotify( Character *, bool, bool );
	virtual bool spec( );

	XML_VARIABLE XMLInteger state;

protected:
	virtual void config( PCharacter * );

private:
	bool spec_hunt_prince( );
	bool spec_kidnap( );
	bool spec_fight( );
	bool spec_sleep( );

	NPCharacter * prince;

	NPCharacter * getPrince( );
	NPCharacter * getPrinceWorld( );
	bool ourPrince( Character * );
	bool princeHere( );
	void princeAttach( );
	void princeDetach( );
	void princeHunt( );
	void princeKidnap( );

	bool heroAttack( PCharacter * = NULL );

	bool canEnter( Room *const );
};


#endif


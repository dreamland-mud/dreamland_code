/* $Id: kidnapmobile.h,v 1.1.2.4.24.2 2008/03/06 17:48:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef KIDNAPMOBILE_H
#define KIDNAPMOBILE_H

#include "mobquestbehavior.h"

class KidnapQuest;

class KidnapMobile : public DedicatedMobile<KidnapQuest> {
XML_OBJECT
public:
	typedef ::Pointer<KidnapMobile> Pointer;
	
protected:
	bool ourKing( Character * );
	NPCharacter * getKingRoom( );
	NPCharacter * getKingRoom( Room * );
	Character * getAggrRoom( Room * );

	void debug( const DLString & );
};


#endif


/* $Id: outsider.h,v 1.1.6.1.10.1 2007/06/26 07:10:43 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef OUTSIDER_H 
#define OUTSIDER_H 

#include "clanmobiles.h"

class ClanGuardOutsider: public ClanGuard {
XML_OBJECT
public:
	typedef ::Pointer<ClanGuardOutsider> Pointer;
    
protected:	
	virtual void actPush( PCharacter * );
	virtual void actGreet( PCharacter * );
	virtual void actIntruder( PCharacter * );
};

#endif

/* $Id: flowers.h,v 1.1.2.2.6.1 2007/06/26 07:10:42 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef FLOWERS_H
#define FLOWERS_H

#include "clanmobiles.h"

class ClanGuardFlowers: public ClanGuard {
XML_OBJECT
public:
	typedef ::Pointer<ClanGuardFlowers> Pointer;
    
protected:	
	virtual bool checkPush( PCharacter * );
	virtual void actPush( PCharacter * );
	virtual void actGreet( PCharacter * );
};

#endif

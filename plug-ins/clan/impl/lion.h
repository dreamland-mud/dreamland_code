/* $Id: lion.h,v 1.1.6.3.6.3 2007/09/11 00:33:58 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef LION_H 
#define LION_H 

#include "objectbehavior.h"
#include "clanmobiles.h"

class ClanGuardLion: public ClanGuard {
XML_OBJECT
public:
	typedef ::Pointer<ClanGuardLion> Pointer;
    
protected:	
	virtual void actPush( PCharacter * );
	virtual void actGreet( PCharacter * );
	virtual int getCast( Character * );
	virtual bool specFight( );
};

class LionEyedSword : public ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<LionEyedSword> Pointer;

    virtual void wear( Character * );
    virtual void equip( Character * );                           
};

#endif

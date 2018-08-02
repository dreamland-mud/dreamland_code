/* $Id: gangmob.h,v 1.1.2.2.6.3 2008/05/16 00:26:24 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef GANGMOB_H
#define GANGMOB_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmllonglong.h"

#include "basicmobilebehavior.h"
#include "wanderer.h"

class Gangsters;
class Room;

class GangMob : public BasicMobileDestiny {
XML_OBJECT    
public:    
    typedef ::Pointer<GangMob> Pointer;
    
    GangMob( );
    virtual ~GangMob( );
    
    virtual void entry( );
    virtual void config( int );
};

class GangMember : public GangMob, public Wanderer {
XML_OBJECT    
public:    
    typedef ::Pointer<GangMember> Pointer;

    enum {
	STAT_NORMAL,
	STAT_FIGHTING,
	STAT_FLEE,
	STAT_TRACKING,
	STAT_SLEEP,
    };
    
    GangMember( );

    virtual bool spec( );
    virtual void bribe( Character *, int, int );
    virtual void fight( Character * );
    virtual bool death( Character * );
    virtual void greet( Character * );

protected:
    virtual bool canEnter( Room *const );
    virtual bool canWander( Room *const, EXTRA_EXIT_DATA * );
    virtual bool canWander( Room *const, Object * );
    virtual int  moveOneStep( int );
    bool makeSomeSteps( int );
    bool runaway( );

    bool eatKey( );
    bool canAttack( Character * );
    void yell( Character *, const DLString& );
    PCharacter * getFightingRoom( Room *const );
    PCharacter * getFightingRoom( );

    XML_VARIABLE XMLString fighting;
    XML_VARIABLE XMLInteger meetCnt;
    XML_VARIABLE XMLBoolean confessed;
    XML_VARIABLE XMLInteger state;
};

#endif

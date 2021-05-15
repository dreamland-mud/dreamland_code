/* $Id: shalafi.h,v 1.1.6.1.10.2 2007/06/26 07:10:43 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef SHALAFI_H 
#define SHALAFI_H 

#include "xmlmap.h"
#include "xmlglobalbitvector.h"
#include "clanmobiles.h"
#include "defaultclan.h"

class ClanGuardShalafi: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardShalafi> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual int getCast( Character * );
};

class ShalafiDemon : public ClanSummonedCreature {
XML_OBJECT
public:
    typedef ::Pointer<ShalafiDemon> Pointer;

    virtual void conjure( );
};

class ShalafiFaculty : public ClanOrder {
XML_OBJECT
public:
    ShalafiFaculty();
    virtual bool canInduct( PCMemoryInterface * ) const;

    XML_VARIABLE XMLGlobalBitvector classes;
};
class ShalafiClan : public DefaultClan {
XML_OBJECT
public:
    typedef ::Pointer<ShalafiClan> Pointer;

    virtual bool canInduct( PCharacter * ) const;
    virtual void onInduct(PCharacter *) const;
};

#endif

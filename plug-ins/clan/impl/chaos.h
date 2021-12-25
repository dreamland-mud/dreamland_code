/* $Id: chaos.h,v 1.1.6.2.6.2 2007/06/26 07:10:42 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef CHAOS_H 
#define CHAOS_H 

#include "objectbehaviormanager.h"
#include "clanmobiles.h"

class ClanGuardChaos: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardChaos> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual int getCast( Character * );
};

class ChaosBlade : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<ChaosBlade> Pointer;

    virtual bool death( Character * );
    virtual void fight( Character * );
};

#endif

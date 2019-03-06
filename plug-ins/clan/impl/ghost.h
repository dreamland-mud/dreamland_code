/* $Id: ghost.h,v 1.1.6.1.10.2 2007/06/26 07:10:42 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef GHOST_H 
#define GHOST_H 

#include "clanmobiles.h"

class ClanGuardGhost: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardGhost> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual bool spec_cast( Character * );
        virtual int getCast( Character * );
};

#endif

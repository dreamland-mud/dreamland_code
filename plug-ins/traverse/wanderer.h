/* $Id: wanderer.h,v 1.1.2.3.6.2 2008/04/14 19:40:54 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef __WANDERER_H__ 
#define __WANDERER_H__ 

#include "mobilebehavior.h"
// MOC_SKIP_BEGIN
#include "roomtraverse.h"
// MOC_SKIP_END


class Wanderer : public virtual MobileBehavior {
public:
        typedef ::Pointer<Wanderer> Pointer;
    
        Wanderer( );

        virtual bool canWander( Room *const, int );
        virtual bool canWander( Room *const, EXIT_DATA * );
        virtual bool canWander( Room *const, EXTRA_EXIT_DATA * );
        virtual bool canWander( Room *const, Object * );

        bool makeSpeedwalk( Room *, Room *, ostringstream & );

protected:
        virtual bool canEnter( Room *const );

        void pathToTarget( Room *const, Room *const, int );
        void pathWithDepth( Room *const, int, int );
        
        bool makeOneStep( Road & );
        void makeOneStep( );
        virtual int moveOneStep( int );
        virtual int moveOneStep( EXTRA_EXIT_DATA * );
        virtual int moveOneStep( Object * );
        virtual bool handleMoveResult( Road &, int );
        
        RoomTraverseResult path;
};


#endif


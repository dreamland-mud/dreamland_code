/* $Id: algorithms.h,v 1.1.2.2 2005/04/27 18:46:16 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef LOCATEQUEST_ALGORITHMS
#define LOCATEQUEST_ALGORITHMS

#include <vector>

#include "dlobject.h"

class Room;
class PCharacter;

class LocateAlgo : public virtual DLObject {
public:
    typedef std::vector<Room *> Rooms;

    virtual void findRooms( PCharacter *, Room *, Room *, Rooms & ) const = 0;
    virtual bool needsEndPoint( ) const = 0;
};

class LocateMiddlePointAlgo : public virtual LocateAlgo {
public:
    virtual void findRooms( PCharacter *, Room *, Room *, Rooms & ) const;
    virtual bool needsEndPoint( ) const;
    virtual int getRadius( ) const;
};

class LocateRadialAlgo : public virtual LocateAlgo {
public:

    virtual void findRooms( PCharacter *, Room *, Room *, Rooms & ) const;
    virtual bool needsEndPoint( ) const;
    virtual int getRadius( ) const;
};

class LocateUniformAlgo: public virtual LocateAlgo {
public:

    virtual void findRooms( PCharacter *, Room *, Room *, Rooms & ) const;
    virtual bool needsEndPoint( ) const;
};

#endif

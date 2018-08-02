/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ROOMBEHAVIOR_H__
#define __ROOMBEHAVIOR_H__

#include "xmlvariablecontainer.h"
#include "xmlpersistent.h"

class Room;
class Character;

class RoomBehavior : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<RoomBehavior> Pointer;
    
    RoomBehavior( );
    virtual ~RoomBehavior( );

    virtual void setRoom( Room * );
    virtual void unsetRoom( );
    Room * getRoom( );

    virtual bool command( Character *, const DLString &, const DLString & );
    virtual bool isCommon( );
    virtual bool canEnter( Character * );

    static const DLString NODE_NAME;

protected:
    Room *room;
};

extern template class XMLStub<RoomBehavior>;

#endif

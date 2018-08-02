/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ROOMCHANNEL_H__
#define __ROOMCHANNEL_H__

#include "globalchannel.h"

class RoomChannel : public GlobalChannel {
XML_OBJECT    
public:
    typedef ::Pointer<RoomChannel> Pointer;

    RoomChannel( );

protected:
    virtual void findListeners( Character *, Listeners & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;
};

#endif

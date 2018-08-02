/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __WORLDCHANNEL_H__
#define __WORLDCHANNEL_H__

#include "globalchannel.h"

class WorldChannel : public GlobalChannel {
XML_OBJECT    
public:
    typedef ::Pointer<WorldChannel> Pointer;
    
    WorldChannel( );

protected:
    virtual void findListeners( Character *, Listeners & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;
};

#endif

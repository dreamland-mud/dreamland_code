/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __AREACHANNEL_H__
#define __AREACHANNEL_H__

#include "worldchannel.h"

class AreaChannel : public WorldChannel {
XML_OBJECT    
public:
    typedef ::Pointer<AreaChannel> Pointer;
    
    AreaChannel( );

protected:
    virtual bool isGlobalListener( Character *, Character * ) const;
    virtual void triggers( Character *, const DLString & ) const;
    virtual void postOutput( Character *outputTo, const DLString &message ) const;
};

#endif

/* $Id: groupchannel.h,v 1.1.2.1 2008/04/04 21:29:02 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __GROUP_H__ 
#define __GROUP_H__ 

#include "commandplugin.h"
#include "globalchannel.h"

class GroupChannel : public GlobalChannel, public CommandPlugin {
XML_OBJECT    
public:
    typedef ::Pointer<GroupChannel> Pointer;

    GroupChannel( );
    virtual ~GroupChannel( );

protected:
    virtual void postOutput( Character *outputTo, const DLString &message ) const;
    virtual void findListeners( Character *, Listeners & ) const;
    virtual bool isGlobalListener( Character *, Character * ) const;
    virtual void triggers( Character *, const DLString & ) const;
    virtual bool canTalkGlobally( Character * ) const;

    static const DLString COMMAND_NAME;
};

#endif

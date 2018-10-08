/* $Id: gquestnotifyplugin.h,v 1.1.2.1 2005/09/10 21:13:00 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef GQUESTNOTIFYPLUGIN_H
#define GQUESTNOTIFYPLUGIN_H

#include "descriptorstatelistener.h"

class GQuestNotifyPlugin: public DescriptorStateListener {
public:
        typedef ::Pointer<GQuestNotifyPlugin> Pointer;
        
        virtual void run( int, int, Descriptor * );
        
};

#endif

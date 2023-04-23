/* $Id$
 *
 * ruffina, 2004
 */
#include "so.h"
#include "mocregistrator.h"
#include "xmlattributeplugin.h"

#include "wrapperhandler.h"
#include "nannyhandler.h"
#include "interprethandler.h"
#include "pagerhandler.h"
#include "serversocket.h"
#include "serversocketcontainer.h"
#include "iomanager.h"
#include "defaultbufferhandler.h"
#include "descriptorstatemanager.h"
#include "webprompt.h"
#include "webmanip.h"
#include "backdoorhandler.h"
#include "badnames.h"
#include "lasthost.h"
#include "ban.h"

extern "C" {
    
    SO::PluginList initialize_iomanager( ) 
    {
        SO::PluginList ppl;

#ifdef __MINGW32__
        WSADATA wsaData;
        WSAStartup(MAKEWORD(1,1), &wsaData); /*is this a right place? ;)*/
#endif  

        Plugin::registerPlugin<InputHandlerRegistrator<InterpretHandler> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<WrapperHandler> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<NannyHandler> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<PagerHandler> >( ppl );
        Plugin::registerPlugin<InputHandlerRegistrator<BackdoorHandler> >( ppl );
        Plugin::registerPlugin<MocRegistrator<ServerSocket> >( ppl );
        Plugin::registerPlugin<ServerSocketContainer>( ppl );
        Plugin::registerPlugin<DefaultBufferHandlerPlugin>( ppl );
        Plugin::registerPlugin<IOManager>( ppl );
        Plugin::registerPlugin<DescriptorStateManager>( ppl );
        Plugin::registerPlugin<WebPromptManager>( ppl );
        Plugin::registerPlugin<WebManipManager>( ppl );
        Plugin::registerPlugin<BadNames>( ppl );
        Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeLastHost> >(ppl);
        Plugin::registerPlugin<XMLAttributeLastHostListenerPlugin>( ppl );
        Plugin::registerPlugin<BanManager>( ppl );
        
        return ppl;
    }
}




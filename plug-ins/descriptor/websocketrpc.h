/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBSOCKETRPC_H
#define WEBSOCKETRPC_H

#include "xmlvariablecontainer.h"


class WebSocketMessage : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<WebSocketMessage> Pointer;
};

class WebSocketHello : public XMLVariableContainer {
XML_OBJECT
public:

    XML_VARIABLE XMLString body;
};



#endif

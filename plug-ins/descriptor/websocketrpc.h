/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBSOCKETRPC_H
#define WEBSOCKETRPC_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"

class Character;
class Descriptor;

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

/** Check if this player is using web client. */
bool is_websock( Character *ch );
bool is_websock( Descriptor * );
/** Create a secure clickable action link for web-client. */
DLString web_cmd(Character *ch, const DLString &cmd, const DLString &seeFmt);
/** Create an Edit button visible to players with elevated security. */
DLString web_edit_button(Character *ch, const DLString &editor, const DLString &args);
DLString web_cmd(Descriptor *d, const DLString &cmd, const DLString &seeFmt);


#endif

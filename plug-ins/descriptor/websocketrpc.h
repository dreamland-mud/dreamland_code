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
class StringList;
namespace Json { class Value; }

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
/** Create a clickable action link with a placeholder in place of nonce. */
DLString web_cmd_placeholder(const DLString &_cmd, const DLString &_seeFmt, const DLString &placeholder);
/** Create an Edit button visible to players with elevated security. */
DLString web_edit_button(Character *ch, const DLString &editor, const DLString &args);
DLString web_cmd(Descriptor *d, const DLString &cmd, const DLString &seeFmt);

/** Create a menu drop-down with given list of commands. */
DLString web_menu(const StringList &commands, const DLString &id, const DLString &label);

/**
 * Structured copy of a list that a command prints, for web clients.
 * Sent as {"command": "list", "args": [list]} next to the usual text, so a client
 * can build a menu without parsing the text in every language. Only clients that
 * sent list_subscribe (OOB_LIST) get it; check active() to skip building rows
 * for everyone else. Format: wiki page Client-Web-Protocol.
 */
class WebList {
public:
    WebList(Character *ch, const DLString &kind, const DLString &title = DLString::emptyString);
    ~WebList();

    bool active() const { return list != 0; }
    /** Add a row and return its index; colour codes are stripped from label and section. */
    int row(const DLString &label, const DLString &id = DLString::emptyString,
            const DLString &section = DLString::emptyString);
    /** Add an action to a row: an English verb from the spec and the command it sends. */
    void action(int row, const DLString &verb, const DLString &cmd);
    /** Raw values for a row, under "data". */
    void data(int row, const DLString &key, int value);
    void data(int row, const DLString &key, bool value);
    void data(int row, const DLString &key, const DLString &value);
    /** Write the frame once; does nothing for non-web clients. */
    void send();

private:
    WebList(const WebList &);
    WebList &operator=(const WebList &);

    Character *ch;
    Json::Value *list;
};

#endif

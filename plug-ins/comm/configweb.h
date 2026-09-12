/* Settings dialog support for the web client.
 *
 * ruffina, 2018 (web prompt), config schema 2026
 */
#ifndef __CONFIGWEB_H__
#define __CONFIGWEB_H__

#include "webprompt.h"

/** Puts the current value of every setting into the web prompt, under 'config'.
 *  The prompt only carries a field when it changes, so an open settings dialog
 *  follows whatever the player does -- including typing 'config brief' into the
 *  terminal, or a Discord bot finishing a link. */
class ConfigWebPromptListener : public WebPromptListener {
public:
    typedef ::Pointer<ConfigWebPromptListener> Pointer;

    virtual void run( Descriptor *, Character *, Json::Value &json );
};

#endif

/* Settings dialog support for the web client. */
#ifndef __CONFIGWEB_H__
#define __CONFIGWEB_H__

#include "descriptorstatelistener.h"
#include "dlstring.h"

class PCharacter;

namespace Json {
    class Value;
}

/** One setting now has another value: tell the client that asked for the schema,
 *  so a switch moved at the keyboard moves in an open dialog too. A client that
 *  never asked is never written to. */
void web_config_changed( PCharacter *ch, const DLString &key, const Json::Value &value );

/** The same, for every other online character of this account: an account-wide
 *  option changed on one of them changes on all of them. */
void web_config_changed_account( const DLString &id, const DLString &key,
                                 const Json::Value &value, PCharacter *except );

/** A character entering or leaving the world: the dialog has nothing to show
 *  for a descriptor sitting at the login prompt, and the settings on screen
 *  belong to whoever is playing now. */
class ConfigWebStateListener : public DescriptorStateListener {
public:
    typedef ::Pointer<ConfigWebStateListener> Pointer;

    virtual void run( int, int, Descriptor * );
};

#endif

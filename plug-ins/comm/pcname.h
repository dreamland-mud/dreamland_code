/* Name-form moderation queue: a per-language name form too far from the login
 * waits here for an immortal's 'nameapprove'. One request per player, kept in
 * the player's XML attribute sidecar so it survives a reboot.
 */
#ifndef PCNAME_H
#define PCNAME_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmldate.h"
#include "xmlboolean.h"
#include "xmlattribute.h"
#include "descriptorstatelistener.h"

class PCharacter;

class XMLAttributeNameForm : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeNameForm> Pointer;

    /** True while no immortal has ruled on the request yet. */
    bool isPending( ) const { return responsible.getValue( ).empty( ); }

    /** Tell the player the verdict and drop the attribute. */
    void deliver( PCharacter * );

    XML_VARIABLE XMLString lang;        // en | ru | ua
    XML_VARIABLE XMLString form;        // full form as it will be stored (Flexer pad for ru/ua)
    XML_VARIABLE XMLDate   date;
    XML_VARIABLE XMLString responsible; // immortal who ruled; empty = pending
    XML_VARIABLE XMLString reason;
    XML_VARIABLE XMLBoolean accepted;
};

/** On entering the game: deliver a verdict reached while the player was away,
 *  and remind immortals of requests still waiting. */
class NameFormLoginListener : public DescriptorStateListener {
public:
    typedef ::Pointer<NameFormLoginListener> Pointer;

    virtual void run( int, int, Descriptor * );
};

#endif

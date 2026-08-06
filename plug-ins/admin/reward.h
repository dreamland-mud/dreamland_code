#ifndef REWARD_H
#define REWARD_H

#include "xmlvector.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlvariablecontainer.h"

#include "descriptorstatelistener.h"
#include "xmlattribute.h"
#include "xmlattributeplugin.h"

class PCharacter;
class XMLReward;

/**
 * Shared body of the 'ireward' command and of '/admin reward', writing its output
 * to a buffer so that both an immortal's screen and a bot response can carry it.
 * Works on the player's memory record, so the target need not be online: the
 * reward is stored as an attribute and handed over on their next login.
 */
void reward_action(const DLString &arguments, std::ostringstream &buf);

class XMLGodReward : public XMLVariableContainer {
XML_OBJECT
public: 
        typedef ::Pointer<XMLGodReward> Pointer;
        
        XMLGodReward( );
        virtual ~XMLGodReward( );

        XML_VARIABLE XMLInteger qp;
        XML_VARIABLE XMLString  reason;
};

class XMLAttributeGodReward : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT;
public:
        typedef ::Pointer<XMLAttributeGodReward> Pointer;

        void addReward( int qp, const DLString &reason );
        void listRewards(ostringstream &buf) const;
        void reward( PCharacter * );
        bool isEmpty() const;

private:        
        XML_VARIABLE XMLVectorBase<XMLGodReward> rewards;
};


class XMLAttributeGodRewardListenerPlugin : public DescriptorStateListener {
public:
        typedef ::Pointer<XMLAttributeGodRewardListenerPlugin> Pointer;

        virtual void run( int, int, Descriptor * );        
};

#endif

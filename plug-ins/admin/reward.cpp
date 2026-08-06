#include <sstream>

#include "reward.h"
#include "admincommand.h"

#include "class.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "descriptor.h"
#include "act.h"
#include "arg_utils.h"
#include "replay.h"
#include "l10n.h"

static const DLString ATTRNAME = "godreward";

XMLGodReward::XMLGodReward( )
{
}

XMLGodReward::~XMLGodReward( )
{
}

void XMLAttributeGodReward::addReward( int qp, const DLString &reason ) 
{
    XMLGodReward r;
    r.qp = qp;
    r.reason = reason;
    rewards.push_back( r );    
}

void XMLAttributeGodReward::listRewards(ostringstream &buf) const
{
    XMLVectorBase<XMLGodReward>::const_iterator r;

    for (r = rewards.begin( ); r!= rewards.end( ); r++) {
        buf << "      " << fmt(0, _("{Y%1$d{x квестов%1$Iую|ые|ых единиц%1$Iу|ы|"), r->qp)
            << "  " << r->reason << endl;
    }
}

void XMLAttributeGodReward::reward( PCharacter *ch ) 
{
    XMLVectorBase<XMLGodReward>::const_iterator r;
    ostringstream buf;

    buf << "{CВ благодарность от богов ты получаешь:{x" << endl;
    listRewards(buf);
    ch->send_to(buf);
    remember_history_private(ch, buf.str());
    
    for (r = rewards.begin( ); r!= rewards.end( ); r++) {
        ch->addQuestPoints(r->qp);
    }

    rewards.clear();
}

bool XMLAttributeGodReward::isEmpty() const
{
    return rewards.empty();
}

void XMLAttributeGodRewardListenerPlugin::run( int oldState, int newState, Descriptor *d ) 
{
    XMLAttributeGodReward::Pointer attr;
    Character *ch = d->character;

    if (!ch)
        return;
    
    if (newState != CON_PLAYING) 
        return;
   
    PCharacter *pch = ch->getPC(); 
    attr = pch->getAttributes( ).findAttr<XMLAttributeGodReward>( ATTRNAME );
    
    if (!attr)
        return;

    attr->reward( pch );
    pch->getAttributes().eraseAttribute(ATTRNAME);
    pch->save();
}


void reward_action(const DLString &constArguments, ostringstream &buf)
{
    PCMemoryInterface *pci;
    DLString arguments = constArguments;
    DLString name = arguments.getOneArgument( );
    DLString qpStr = arguments.getOneArgument( );

    if (name.empty( ) || qpStr.empty( )) {
        buf << "Usage: reward <player> <qp> <reason>" << endl;
        buf << "Usage: reward <player> show" << endl;
        buf << "Usage: reward <player> del" << endl;
        return;
    }

    if (!( pci = PCharacterManager::find( name ) )) {
        buf << "Player " << name << " not found." << endl;
        return;
    }

    if (arg_is(qpStr, "del")) {
        pci->getAttributes().eraseAttribute(ATTRNAME);
        PCharacterManager::saveMemory(pci);
        buf << "All pending rewards for " << pci->getName() << " removed." << endl;
        return;
    }

    XMLAttributeGodReward::Pointer attr = pci->getAttributes().getAttr<XMLAttributeGodReward>(ATTRNAME);

    if (arg_is_show(qpStr)) {
        if (attr->isEmpty()) {
            buf << "No pending rewards for " << pci->getName() << "." << endl;
            return;
        }

        buf << "Награды для персонажа " << pci->getName() << ":" << endl;
        attr->listRewards(buf);
        return;
    }

    int qp;
    try {
        qp = qpStr.toInt( );
    } catch (const ExceptionBadType& e) {
        buf << "Quest point amount expected, got '" << qpStr << "'." << endl;
        return;
    }

    attr->addReward(qp, arguments);
    buf << "Reward of " << qp << " qp set for " << pci->getName()
        << ", reason: " << arguments << "." << endl;

    // Online players get it straight away; everyone else on their next login.
    if (pci->isOnline()) {
        PCharacter *vict = pci->getPlayer();
        attr->reward(vict);
        vict->getAttributes().eraseAttribute(ATTRNAME);
        vict->save();
    } else {
        PCharacterManager::saveMemory(pci);
        buf << "Player is offline, the reward waits for their next login." << endl;
    }
}

CMDADM( ireward )
{
    ostringstream buf;

    reward_action( constArguments, buf );
    ch->send_to( buf );
}
    

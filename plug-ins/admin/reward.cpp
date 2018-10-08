#include <sstream>

#include "reward.h"
#include "admincommand.h"

#include "class.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "descriptor.h"
#include "act.h"
#include "arg_utils.h"

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
        buf << "      " << fmt(0, "{Y%1$d{x квестов%1$Iую|ые|ых единиц%1$Iу|ы|", r->qp)
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

    for (r = rewards.begin( ); r!= rewards.end( ); r++) {
        ch->questpoints += r->qp;
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


CMDADM( ireward )
{
    PCMemoryInterface *pci;
    DLString arguments = constArguments; 
    DLString name = arguments.getOneArgument( );
    DLString qpStr = arguments.getOneArgument( );
    DLString reason = arguments;

    if (name.empty( )) {
	ch->println( "Вознаградить кого?" );
	return;
    }

    if (qpStr.empty( )) {
        ch->println("Использование: \r\n    ireward персонаж кп причина\r\n    ireward персонаж show\r\n    ireward персонаж delete");
        return;
    }

    if (!( pci = PCharacterManager::find( name ) )) {
	ch->println( "Персонаж с таким именем не найден." );
	return;
    }

    if (arg_oneof(qpStr, "удалить", "delete")) {
        pci->getAttributes().eraseAttribute(ATTRNAME);
        PCharacterManager::saveMemory(pci);
        ch->println("Удалены все награды.");
        return;
    } 

    XMLAttributeGodReward::Pointer attr = pci->getAttributes().getAttr<XMLAttributeGodReward>(ATTRNAME);

    if (arg_is_show(qpStr)) {
        if (attr->isEmpty()) {
            ch->println("Наград не найдено.");
            return;
        }

        ostringstream buf;
        buf << "Награды для персонажа " << pci->getName() << ":" << endl;
        attr->listRewards(buf);
        ch->send_to(buf);
        return;
    } 


    int qp;
    try {
        qp = qpStr.toInt( );
    } catch (const ExceptionBadType& e) {
        ch->println("Укажите кол-во квестовых единиц.\r\n");
        return;
    }

    attr->addReward(qp, arguments);
    ch->printf("Установлена награда в %d qp, причина: %s.\r\n", qp, arguments.c_str());

    if (pci->isOnline()) {      
        PCharacter *vict = pci->getPlayer();
        attr->reward(vict);
        vict->getAttributes().eraseAttribute(ATTRNAME);
        vict->save();
    } else {
        PCharacterManager::saveMemory(pci);
    }
}
    

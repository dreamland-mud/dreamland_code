/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __GLOBALCHANNEL_H__
#define __GLOBALCHANNEL_H__

#include <list>
#include "communicationchannel.h"

class GlobalChannel : public virtual CommunicationChannel {
XML_OBJECT    
public:
    typedef ::Pointer<GlobalChannel> Pointer;
    typedef list<Character *> Listeners;
    
    GlobalChannel( );

    virtual void run( Character *, const DLString & );

protected:
    virtual bool canTalkGlobally( Character * ) const;
    virtual void findListeners( Character *, Listeners & ) const = 0;
    virtual bool isGlobalListener( Character *, Character * ) const;
    virtual void triggers( Character *, const DLString & ) const;

    virtual bool checkNoChannel( Character * ) const;
    virtual bool checkConfirmed( Character * ) const;
    virtual bool checkSoap( Character * ) const;

    virtual bool needOutputSelf( Character * ) const;
    virtual bool needOutputOther( Character * ) const;

    XML_VARIABLE XMLBooleanNoFalse soap, translate, nochannel, deafenOther;
    XML_VARIABLE XMLBooleanNoFalse quiet, nomob, confirmed, dig;

    XML_VARIABLE XMLStringNoEmpty msgSelf, msgOther, msgSelfNoarg, msgOtherNoarg;
    XML_VARIABLE XMLStringNoEmpty msgSelfMild, msgOtherMild;
    XML_VARIABLE XMLStringNoEmpty msgNochan, msgListEmpty;
    XML_VARIABLE XMLStringNoEmpty msgOn, msgOff;
    XML_VARIABLE XMLStringNoEmpty msgDisable;

    XML_VARIABLE XMLIntegerNoEmpty manaPercent;
};

#endif

/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __BANKING_H__
#define __BANKING_H__

#include "roombehavior.h"
#include "objectbehaviormanager.h"
#include "descriptorstatelistener.h"

class PCharacter;

class BankAction : public virtual DLObject {
public:

protected:
    bool handleCommand( Character *, const DLString &, const DLString & );
    void doBalance( PCharacter * );
    void doWithdraw( PCharacter *, DLString & );
    void doDeposit( PCharacter *, DLString & );
};

class BankRoom : public BankAction, public virtual RoomBehavior {
XML_OBJECT
public:
    typedef ::Pointer<BankRoom> Pointer;
    
    virtual bool command( Character *, const DLString &, const DLString & );
};

class CreditCard : public BankAction, public virtual BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<CreditCard> Pointer;

    virtual bool command( Character *, const DLString &, const DLString & );
    virtual bool mayFloat( ) { return true; }
    virtual bool canConfiscate( ) { return false; }
    virtual bool hasTrigger( const DLString &  );
};

class TaxesListener : public DescriptorStateListener {
public:
    typedef ::Pointer<TaxesListener> Pointer;
    
    virtual void run( int, int, Descriptor * );
};

#endif

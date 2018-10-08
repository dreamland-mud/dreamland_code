/* $Id: remortnanny.h,v 1.1.2.8.4.2 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef REMORTNANNY_H
#define REMORTNANNY_H

#include "servicetrader.h"
#include "descriptorstatelistener.h"

class PCharacter;
class Remorts;

class RemortNanny : public DescriptorStateListener {
public:
    typedef ::Pointer<RemortNanny> Pointer;
    
    virtual void run( int, int, Descriptor * );
    
};

class RemortWitch : public ServiceTrader, 
                    public Seller, 
                    public TraderBehavior
{
XML_OBJECT
public:
    typedef ::Pointer<RemortWitch> Pointer;

    RemortWitch( );

    virtual void greet( Character *victim );
    virtual void speech( Character *victim, const char *speech );
    virtual void tell( Character *victim, const char *speech );
    virtual bool look_inv( Character *looker );

protected:
    virtual bool canServeClient( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );
    virtual void msgListEmpty( Character * ) { }
    virtual void msgListBefore( Character * );
    virtual void msgListAfter( Character * ) { }
    virtual void msgListRequest( Character * ) { }
    
    virtual void msgBuyRequest( Character * ) { }

    virtual void msgArticleNotFound( Character * );
    virtual void msgProposalNotFound( Character * );
};

#endif


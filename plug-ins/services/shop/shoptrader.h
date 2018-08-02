/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SHOPTRADER_H__
#define __SHOPTRADER_H__

#include "trader.h"
#include "repairman.h"

class ShopTrader :  public Repairman {
XML_OBJECT
public:
    typedef ::Pointer<ShopTrader> Pointer;
    
    ShopTrader( );

    virtual void load( DLString );
    virtual void give( Character *, Object * );
    virtual int getOccupation( );
    virtual void speech( Character *victim, const char *speech );
    virtual void tell ( Character *victim, const char *speech );
    
#if 0
    virtual void doSell( PCharacter *, const DLString & );
    virtual void doValue( PCharacter *, const DLString & );
    
protected:
    virtual bool canServeClient( PCharacter * );
    virtual Article::Pointer findArticle( PCharacter *, DLString & );
    virtual void toStream( PCharacter *, ostringstream & );

    virtual void msgListEmpty( PCharacter * );
    virtual void msgListBefore( PCharacter * );
    virtual void msgListAfter( PCharacter * );
    virtual void msgListRequest( PCharacter * );
    virtual void msgBuyRequest( PCharacter * );
    virtual void msgArticleNotFound( PCharacter * );
    virtual void msgArticleTooFew( PCharacter *, Article::Pointer );
#endif
    XML_VARIABLE XMLInteger closeHour, openHour;
    XML_VARIABLE XMLInteger profitBuy, profitSell;
    XML_VARIABLE XMLFlagsNoEmpty buys;

    void describeGoods( Character *client, const DLString &arg, bool verbose );
};

#endif

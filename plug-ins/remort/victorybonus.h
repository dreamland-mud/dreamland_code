/* $Id$
 *
 * ruffina, 2004
 */
#ifndef VICTORYBONUS_H
#define VICTORYBONUS_H

#include "servicetrader.h"
#include "price.h"

class Koschey : public ServiceTrader, public TraderBehavior {
XML_OBJECT
public:
    typedef ::Pointer<Koschey> Pointer;

    virtual void greet( Character * );
    virtual bool command( Character *, const DLString &, const DLString & );

protected:
    virtual bool canServeClient( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );
    virtual void msgListEmpty( Character * );
    virtual void msgListBefore( Character * );
    virtual void msgListAfter( Character * );
    virtual void msgListRequest( Character * ) { }
    
    virtual void msgBuyRequest( Character * );

    virtual void msgArticleNotFound( Character * );
};

class VictoryPrice : public Price, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<VictoryPrice> Pointer;

    virtual bool canAfford( Character * ) const;
    virtual void induct( Character * ) const;
    virtual void deduct( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;
    
    virtual DLString toCurrency( ) const;
    virtual DLString toString( Character * ) const;

    static const int COUNT_PER_LIFE;

protected:
    XML_VARIABLE XMLInteger count;

    static const DLString CURRENCY_NAME;
};

#endif

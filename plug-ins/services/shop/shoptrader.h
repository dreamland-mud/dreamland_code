/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SHOPTRADER_H__
#define __SHOPTRADER_H__

#include "trader.h"
#include "basicmobilebehavior.h"
#include "xmlflags.h"

class ShopTrader :  public virtual BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<ShopTrader> Pointer;
    
    ShopTrader( );

    virtual void load( DLString );
    virtual void give( Character *, Object * );
    virtual int getOccupation( );
    virtual void speech( Character *victim, const char *speech );
    virtual void tell ( Character *victim, const char *speech );
    virtual void show( Character *, std::basic_ostringstream<char> & );
    
    XML_VARIABLE XMLInteger closeHour, openHour;
    XML_VARIABLE XMLInteger profitBuy, profitSell;
    XML_VARIABLE XMLFlagsNoEmpty buys;

    void describeGoods( Character *client, const DLString &arg, bool verbose );
};

#endif

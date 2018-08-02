/* $Id$
 *
 * ruffina, 2004
 */
#ifndef LIFEPRICE_H
#define LIFEPRICE_H

#include "price.h"

class LifePrice : public Price, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<LifePrice> Pointer;

    virtual bool canAfford( Character * ) const;
    virtual void induct( Character * ) const;
    virtual void deduct( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;
    
    virtual DLString toCurrency( ) const;
    virtual DLString toString( Character * ) const;

protected:
    XML_VARIABLE XMLInteger points;

    static const DLString LIFE_ONE, LIFE_MANY;
};

#endif

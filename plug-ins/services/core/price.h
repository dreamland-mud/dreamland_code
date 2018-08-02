/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PRICE_H__
#define __PRICE_H__

#include <sstream>

#include "xmlpolymorphvariable.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"

class Character;


class Price : public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<Price> Pointer;
    
    virtual ~Price( );

    virtual bool canAfford( Character * ) const = 0;
    virtual void deduct( Character * ) const = 0;
    virtual void induct( Character * ) const = 0;
    virtual void toStream( Character *, ostringstream & ) const = 0;
    virtual DLString toCurrency( ) const = 0;
    virtual DLString toString( Character * ) const = 0;
};

class MoneyPrice : public Price {
public:
    typedef ::Pointer<MoneyPrice> Pointer;
    
    virtual bool canAfford( Character * ) const;
    virtual void deduct( Character * ) const;
    virtual void induct( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;
    virtual DLString toCurrency( ) const;
    virtual DLString toString( Character * ) const;

protected:
    virtual void taxes( int ) const;
    virtual int toSilver( Character * ) const = 0;
    virtual int haggle( Character * ) const;

private:
    static const DLString CURRENCY_NAME;
};

class CoinPrice : public MoneyPrice, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<CoinPrice> Pointer;

protected:
    virtual int toSilver( Character * ) const;

    XML_VARIABLE XMLInteger gold, silver;
};

class LevelPrice : public MoneyPrice, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<LevelPrice> Pointer;
    
    LevelPrice( );

protected:
    virtual int getLevel( Character * ) const;
    virtual int toSilver( Character * ) const;

    XML_VARIABLE XMLInteger coef;
    XML_VARIABLE XMLInteger bonus;
    XML_VARIABLE XMLInteger power;
};


class QuestPointPrice : public Price, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<QuestPointPrice> Pointer;

    virtual bool canAfford( Character * ) const;
    virtual void deduct( Character * ) const;
    virtual void induct( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;
    
    virtual DLString toCurrency( ) const;
    virtual DLString toString( Character * ) const;

protected:
    XML_VARIABLE XMLInteger questpoint;

private:
    static const DLString CURRENCY_NAME;
};

#endif

/* $Id$
 *
 * ruffina, 2004
 */
/*
 * TODO
 */
#ifndef __SHOPPRICES_H__
#define __SHOPPRICES_H__

#include "price.h"

class ShopPrice : public MoneyPrice {
public:
    ShopPrice( ShopArticle::Pointer );

protected:
    virtual int toSilver( Character * ) const;
    virtual void taxes( int ) const;
    virtual int calculate( ) = 0;

    int silver;
    ShopArticle::Pointer article;
};

class ShopBuyPrice : public ShopPrice {
public:
    ShopBuyPrice( ShopArticle::Pointer, int );
    
    int getSingleCost( ) const;

protected:
    virtual int toSilver( Character * ) const;
    virtual int haggle( Character * ) const;
    virtual int calculate( );

    int quantity;
};

class ShopSellPrice : public ShopPrice {
public:
    ShopSellPrice( ShopArticle::Pointer );

protected:
    virtual int haggle( Character * ) const;
    virtual int calculate( );
};

#endif

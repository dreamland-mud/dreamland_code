/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SMITHMAN_H__
#define __SMITHMAN_H__

#include "xmlstring.h"
#include "xmlvariablecontainer.h"

#include "commandtemplate.h"
#include "basicmobilebehavior.h"

#include "price.h"
#include "servicetrader.h"

class Smithman : public ServiceTrader, 
                 public TraderBehavior,
                 public BasicMobileDestiny 
{
XML_OBJECT
public:
    typedef ::Pointer<Smithman> Pointer;
    
    virtual int getOccupation( );

protected:
    virtual bool specIdle( );

    virtual bool canServeClient( Character * );
    virtual void msgListEmpty( Character * );
    virtual void msgListBefore( Character * ); 
    virtual void msgListAfter( Character * );
    virtual void msgListRequest( Character * );
    virtual void msgBuyRequest( Character * );
    virtual void msgArticleNotFound( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );
};

class SmithService : public Service, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<SmithService> Pointer;

    virtual void toStream( Character *, ostringstream & ) const;
    virtual bool matches( const DLString & ) const;
    virtual int getQuantity( ) const;

protected:
    static void printLine( Character *, Price::Pointer, const DLString &, const DLString &, ostringstream & );

    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString rname;
    XML_VARIABLE XMLString descr;
    XML_VARIABLE XMLPointer<Price> price;
};

class HorseshoeSmithService : public SmithService {
XML_OBJECT    
public:
    typedef ::Pointer<HorseshoeSmithService> Pointer;
    
    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;
    virtual bool purchase( Character *, NPCharacter *, const DLString &, int = 1 );
};

class ItemSmithService : public SmithService {
XML_OBJECT    
public:
    typedef ::Pointer<ItemSmithService> Pointer;
    
    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;
    virtual bool purchase( Character *, NPCharacter *, const DLString &, int = 1 );

protected:
    bool checkPrice( Character *, NPCharacter *, Price::Pointer ) const;
    virtual void smith( Character *, NPCharacter *, Object * ) = 0;

    XML_VARIABLE XMLString verb;
    XML_VARIABLE XMLString noMoney;
};

class SharpSmithService : public ItemSmithService {
XML_OBJECT    
public:
    typedef ::Pointer<SharpSmithService> Pointer;
    
    virtual void toStream( Character *, ostringstream & ) const;

protected:
    virtual void smith( Character *, NPCharacter *, Object * );

    XML_VARIABLE XMLString extraDescr;
    XML_VARIABLE XMLPointer<Price> extraPrice;
};

class BurnproofSmithService : public ItemSmithService {
XML_OBJECT    
public:
    typedef ::Pointer<BurnproofSmithService> Pointer;
    
protected:
    virtual void smith( Character *, NPCharacter *, Object * );
};

class AlignSmithService : public ItemSmithService {
XML_OBJECT    
public:
    typedef ::Pointer<AlignSmithService> Pointer;
    
protected:
    virtual void smith( Character *, NPCharacter *, Object * );
};

#endif

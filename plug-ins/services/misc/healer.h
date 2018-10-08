/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __HEALER_H__    
#define __HEALER_H__    

#include "xmlstring.h"
#include "xmlboolean.h"
#include "xmlvariablecontainer.h"

#include "skillreference.h"
#include "basicmobilebehavior.h"

#include "price.h"
#include "servicetrader.h"

class Healer : public ServiceTrader, 
               public TraderBehavior, 
               public BasicMobileDestiny 
{
XML_OBJECT
public:
    typedef ::Pointer<Healer> Pointer;
    
    Healer( );
    virtual int getOccupation( );

protected:
    virtual bool canServeClient( Character * );
    virtual void msgListEmpty( Character * );
    virtual void msgListBefore( Character * ); 
    virtual void msgListAfter( Character * );
    virtual void msgListRequest( Character * );
    virtual void msgBuyRequest( Character * );
    virtual void msgArticleNotFound( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );

    XML_VARIABLE XMLBoolean healPets;
};

class HealService : public Service, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<HealService> Pointer;

    virtual void toStream( Character *, ostringstream & ) const;
    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;
    virtual bool matches( const DLString & ) const;
    virtual int getQuantity( ) const;
    virtual void purchase( Character *, NPCharacter *, const DLString &, int = 1 );

protected:
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString descr;
    XML_VARIABLE XMLPointer<Price> price;

private:    
    virtual void heal( Character *, NPCharacter * ) = 0;
};

class SpellHealService : public HealService {
XML_OBJECT    
public:
    typedef ::Pointer<SpellHealService> Pointer;


private:
    virtual void heal( Character *, NPCharacter * );

    XML_VARIABLE XMLSkillReference spell;
};

class ManaHealService : public HealService {
XML_OBJECT    
public:
    typedef ::Pointer<ManaHealService> Pointer;

private:
    virtual void heal( Character *, NPCharacter * );

    XML_VARIABLE XMLBoolean enhanced;
    XML_VARIABLE XMLString words;
};

#endif

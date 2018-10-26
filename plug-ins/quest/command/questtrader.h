/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __QUESTTRADER_H__
#define __QUESTTRADER_H__

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlboolean.h"
#include "xmlinteger.h"
#include "xmlenumeration.h"

#include "servicetrader.h"
#include "basicmobilebehavior.h"
#include "price.h"

class QuestTrader : public ServiceTrader, public TraderBehavior,
                    public virtual BasicMobileDestiny                    
{
XML_OBJECT
public:
    typedef ::Pointer<QuestTrader> Pointer;
    
    virtual void doTrouble( PCharacter *, const DLString & );

    virtual int getOccupation( );

    virtual bool canServeClient( Character * );

protected:
    virtual void msgListEmpty( Character * );
    virtual void msgListBefore( Character * ); 
    virtual void msgListAfter( Character * );
    virtual void msgListRequest( Character * );
    
    virtual void msgBuyRequest( Character * );
    
    virtual void msgArticleNotFound( Character * );
    virtual void msgArticleTooFew( Character *, Article::Pointer );
};

class QuestTradeArticle : public Service, public XMLVariableContainer {
XML_OBJECT    
public:
    typedef ::Pointer<QuestTradeArticle> Pointer;

    virtual void toStream( Character *, ostringstream & ) const;
    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;
    virtual bool matches( const DLString & ) const;
    virtual void purchase( Character *, NPCharacter *, const DLString &, int = 1 );
    virtual int getQuantity( ) const;
   
protected:
    virtual void buy( PCharacter *, NPCharacter * ) = 0;

    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString rname;
    XML_VARIABLE XMLString descr;
    XML_VARIABLE XMLPointer<Price> price;
};

class ObjectQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<ObjectQuestArticle> Pointer;

protected:
    XML_VARIABLE XMLInteger vnum;

    virtual void buy( PCharacter *, NPCharacter * );
    virtual void buyObject( Object *, PCharacter *, NPCharacter * );
};

class ConQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<ConQuestArticle> Pointer;

    virtual bool available( Character *, NPCharacter * ) const;

private:
    virtual void buy( PCharacter *, NPCharacter * );
};

class GoldQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<GoldQuestArticle> Pointer;

protected:
    XML_VARIABLE XMLInteger amount;

private:
    virtual void buy( PCharacter *, NPCharacter * );
};

class PocketsQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<PocketsQuestArticle> Pointer;

    virtual bool available( Character *, NPCharacter * ) const;

protected:
    virtual void buy( PCharacter *, NPCharacter * );
    Object * findBag( PCharacter * ) const;
};

class KeyringQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<KeyringQuestArticle> Pointer;

    virtual bool available( Character *, NPCharacter * ) const;

protected:
    virtual void buy( PCharacter *, NPCharacter * );
};

class PersonalQuestArticle : public ObjectQuestArticle {
XML_OBJECT
public:
    typedef ::Pointer<PersonalQuestArticle> Pointer;
    
    PersonalQuestArticle( );

    virtual void trouble( PCharacter *, NPCharacter * );

protected:
    XML_VARIABLE XMLBoolean troubled;
    XML_VARIABLE XMLEnumeration gender;

    virtual void buyObject( Object *, PCharacter *, NPCharacter * );
};

class OwnerPrice : public Price, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<OwnerPrice> Pointer;

    virtual bool canAfford( Character * ) const;
    virtual void deduct( Character * ) const;
    virtual void induct( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;
    virtual DLString toCurrency( ) const;
    virtual DLString toString( Character * ) const;

    int getValue( PCharacter * ) const;

protected:
    XML_VARIABLE XMLInteger lifes, victories;
    static const DLString LIFE_NAME, VICTORY_NAME;
};

class OwnerQuestArticle : public ObjectQuestArticle {
XML_OBJECT
public:
    typedef ::Pointer<OwnerQuestArticle> Pointer;

    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;

private:
    virtual void buyObject( Object *, PCharacter *, NPCharacter * );
    XML_VARIABLE OwnerPrice lifePrice;
};

class PiercingQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:

    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;

private:
    virtual void buy( PCharacter *, NPCharacter * );
};

class TattooQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<TattooQuestArticle> Pointer;

    virtual bool available( Character *, NPCharacter * ) const;

private:
    virtual void buy( PCharacter *, NPCharacter * );
};

#endif

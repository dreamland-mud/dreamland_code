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
#include "eventbus.h"
#include "plugin.h"

class ItemReadEvent;

/**
 * Backfills the English and Ukrainian names of personalised objects created
 * before the engine learned to write every language slot -- the hero quest
 * items and the hunter clan gear, both of which carry their owner in the
 * 'owner' field and a "%s" template in every language of their prototype.
 *
 * Rides ItemReadEvent, which reaches everything saved: player inventories and
 * room contents alike are re-read from disk, so no separate admin pass is
 * needed. Idempotent -- an object with no empty slot behind a template costs
 * one string check.
 */
class PersonalNameRepair : public Plugin, public EventHandler {
public:
    typedef ::Pointer<PersonalNameRepair> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual void handleEvent( const type_index &eventType, const Event &event ) const;

protected:
    void eventItemRead( const ItemReadEvent &event ) const;
};

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
    virtual bool purchase( Character *, NPCharacter *, const DLString &, int = 1 );
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

/**
 * Sells the 'personal' flavor wearlocation: one purchase forever unlocks an
 * extra slot that holds a single item of any type, worn purely for show (no
 * stats). Ownership is a Character::wearloc bit, same mechanism as the ear
 * piercing below; the slot itself lives in plug-ins/wearlocation.
 */
class WearslotQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<WearslotQuestArticle> Pointer;

    virtual bool visible( Character * ) const;
    virtual bool available( Character *, NPCharacter * ) const;

private:
    virtual void buy( PCharacter *, NPCharacter * );
};

/**
 * Refits a personalised, level-stamped reward item (hero girth/ring/weapon/bag/
 * keyring) to the buyer's current real level, so it becomes wearable again after
 * a remort dropped the character to level 1 while the item stayed stamped near
 * 100. equip() rescales every affect from the new level on the next wear, so a
 * refit item is weak at level 1 and grows back as the owner levels -- self
 * balancing, no extra bookkeeping.
 *
 * Price is dynamic rather than a catalog Price: the bag is always free, a gap of
 * five levels or less is free, otherwise gap * (questTier + 1) qp. Every eligible
 * carried item is refit in one purchase, and the total is charged once.
 */
class RefitQuestArticle : public QuestTradeArticle {
XML_OBJECT
public:
    typedef ::Pointer<RefitQuestArticle> Pointer;

    virtual void toStream( Character *, ostringstream & ) const;
    virtual bool available( Character *, NPCharacter * ) const;
    virtual bool purchase( Character *, NPCharacter *, const DLString &, int = 1 );

protected:
    virtual void buy( PCharacter *, NPCharacter * );
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

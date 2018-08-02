/* $Id: objects.h,v 1.1.2.3.18.3 2007/09/11 00:33:54 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CARDOBJECTS_H
#define CARDOBJECTS_H

#include "xmlstring.h"
#include "xmlinteger.h"
#include "objectbehavior.h"
#include "objectbehaviorplugin.h"

#define OBJ_VNUM_CARD 28200
#define OBJ_VNUM_CARDPACK 28201
#define OBJ_VNUM_CARD_SLEEVES 28202

class Room;
class PCharacter;
class NPCharacter;

class CardPackBehavior : public ObjectBehavior {
XML_OBJECT
public:
	typedef ::Pointer<CardPackBehavior> Pointer;
    
	CardPackBehavior( );

	virtual bool use( Character *, const char * );
	virtual bool examine( Character * );
        virtual bool hasTrigger( const DLString &  );
	
private:
	XML_VARIABLE XMLInteger throws;	
};

class CardBehavior : public ObjectBehavior {
XML_OBJECT
public:
	typedef ::Pointer<CardBehavior> Pointer;
    
	CardBehavior( );

	virtual bool use( Character *, const char *);
	virtual bool examine( Character * );
	virtual bool command( Character *, const DLString &, const DLString & );
	virtual DLString extraDescription( Character *ch, const DLString & );
        virtual bool hasTrigger( const DLString &  );
	
	inline const DLString & getPlayerName( ) const;
	inline void setPlayerName( const DLString & );
	inline int  getQuality( ) const;
	inline void setQuality( int );

private:
	CardBehavior::Pointer findMyCard( PCharacter *, PCharacter * );
	DLString spoilDescription( PCharacter *, const char *, int );
	NPCharacter * findHorribleVictim( PCharacter * );
	Room * findHorribleRoom( PCharacter * );
	
	XML_VARIABLE XMLString playerName;
	XML_VARIABLE XMLInteger quality;
};

inline const DLString & CardBehavior::getPlayerName( ) const
{
    return playerName.getValue( );
}
inline void CardBehavior::setPlayerName( const DLString & name )
{
    playerName = name;
}
inline int CardBehavior::getQuality( ) const
{
    return quality.getValue( );
}
inline void CardBehavior::setQuality( int q )
{
    quality.setValue( q );
}

class CardSleevesBehavior : public ObjectBehavior {
XML_OBJECT
public:
	typedef ::Pointer<CardSleevesBehavior> Pointer;
    
	CardSleevesBehavior( );

	virtual void fight( Character * );
	virtual bool canEquip( Character * );
};

#endif


/* $Id: xmlattributecards.h,v 1.1.2.8.6.2 2007/06/26 07:09:27 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef XMLATTRIBUTECARDS_H
#define XMLATTRIBUTECARDS_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "playerattributes.h"
#include "scheduledxmlattribute.h"

class XMLAttributeCards : 
    public EventHandler<DeathArguments>, 
    public XMLVariableContainer
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeCards> Pointer;

	XMLAttributeCards( );
	virtual ~XMLAttributeCards( );

	virtual bool handle( const DeathArguments & ); 
	
	inline void setContactName( const DLString & );
	inline const DLString & getContactName( ) const;
	inline int getLevel( ) const;
	inline void setLevel( int );
	inline int getSuit( ) const;
	inline void setSuit( int );
	static int getRandomSuit( );
	DLString getFace( char ) const;
	bool isTrump( ) const;
	
	static int getMaxLevel( );
	struct CardLevelFace {
	    const char *name;
	    int gender;
	};
	static const CardLevelFace levelFaces[];

	struct CardSuitFace {
	    const char *mlt;
	    const char *male;
	    const char *female;
	};
	static const CardSuitFace suitFaces[];

	inline const CardLevelFace& getLevelFace( ) const;
	inline const CardSuitFace& getSuitFace( ) const;

	static int getTrump( );
	static inline const CardSuitFace& getTrumpFace( );
	
private:
	XML_VARIABLE XMLInteger level;
	XML_VARIABLE XMLInteger suit;
	XML_VARIABLE XMLString contactName;
};

inline void XMLAttributeCards::setContactName( const DLString &value )
{
    contactName.setValue( value );
}
inline const DLString & XMLAttributeCards::getContactName( ) const
{
    return contactName.getValue( );
}
inline int XMLAttributeCards::getLevel( ) const
{
    return level.getValue( );
}
inline void XMLAttributeCards::setLevel( int level ) 
{
    this->level = level;
}
inline int XMLAttributeCards::getSuit( ) const
{
    return suit.getValue( );
}
inline void XMLAttributeCards::setSuit( int suit ) 
{
    this->suit = suit;
}
inline const XMLAttributeCards::CardLevelFace& XMLAttributeCards::getLevelFace( ) const
{
    return levelFaces[getLevel( )];
}
inline const XMLAttributeCards::CardSuitFace& XMLAttributeCards::getSuitFace( ) const
{
    return suitFaces[getSuit( )];
}
inline const XMLAttributeCards::CardSuitFace& XMLAttributeCards::getTrumpFace( ) 
{
    return suitFaces[getTrump( )];
}

#endif


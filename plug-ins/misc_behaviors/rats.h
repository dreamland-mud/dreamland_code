/* $Id: rats.h,v 1.1.2.8.6.2 2007/09/11 00:32:10 rufina Exp $
 *
 * ruffina, cradya, 2004
 */

#ifndef RAT_H
#define RAT_H

#include "xmlvariablecontainer.h"
#include "xmlboolean.h"
#include "xmlinteger.h"

#include "pet.h"
#include "xmlattribute.h"

class Rat : public LevelAdaptivePet {
XML_OBJECT
public:
	typedef ::Pointer<Rat> Pointer;
    
	Rat( );

	virtual bool death( Character *killer );
	virtual void stopfol( Character *master );
	virtual bool area( );

protected:

	XML_VARIABLE XMLInteger timer;
};

class RatGod : public BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<RatGod> Pointer;
    
	virtual void greet( Character *victim );
	
protected:
	Character * getActor( Character * );
	void exorcism( Character *, const char * );
};

class XMLAttributeRats : public XMLAttribute, public XMLVariableContainer {
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeRats> Pointer;

	XMLAttributeRats( );

	XML_VARIABLE XMLBoolean nongrata;
	XML_VARIABLE XMLBoolean desecrator;
};

#endif


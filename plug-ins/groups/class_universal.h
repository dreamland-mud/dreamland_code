/* $Id: class_universal.h,v 1.1.2.2 2007/09/11 00:34:12 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef __CLASS_UNIVERSAL_H__ 
#define __CLASS_UNIVERSAL_H__ 

#include "scheduledxmlattribute.h"

#include "shoptrader.h"
#include "basicmobilebehavior.h"
#include "mobilebehaviorplugin.h"
#include "xmlattribute.h"
#include "xmlattributeplugin.h"

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlmap.h"
#include "xmlinteger.h"
#include "xmllong.h"

class UniclassAdept : public virtual BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<UniclassAdept> Pointer;
    
	UniclassAdept( );

	virtual void speech( Character *victim, const char *speech );
	virtual void tell( Character *victim, const char *speech );
	
protected:
        bool parseSpeech( Character *victim, const char *speech, DLString &className );
	XML_VARIABLE XMLString myclass;
};

class UniclassAdeptAndShopTrader : public UniclassAdept, public ShopTrader {
XML_OBJECT
public:
	typedef ::Pointer<UniclassAdeptAndShopTrader> Pointer;
	
	virtual ~UniclassAdeptAndShopTrader( );
	virtual void speech( Character *victim, const char *speech );
	virtual void tell( Character *victim, const char *speech );
};

class XMLAttributeUniclass : public XMLAttribute, public XMLVariableContainer
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeUniclass> Pointer;

	XMLAttributeUniclass( );

	XML_VARIABLE XMLLong lastTime;
	XML_VARIABLE XMLMapBase<XMLInteger> history;
};

class DwarkinAdept : public BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<DwarkinAdept> Pointer;
    
	DwarkinAdept( );

	virtual void speech( Character *victim, const char *speech );
	virtual void tell( Character *victim, const char *speech );
	
protected:
	XML_VARIABLE XMLString myclass;
};

class XMLAttributeEnlight : public ScheduledXMLAttribute, public XMLVariableContainer
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeEnlight> Pointer;

	XMLAttributeEnlight( );
	virtual bool pull( PCharacter * );

	XML_VARIABLE XMLInteger age;
};

#endif


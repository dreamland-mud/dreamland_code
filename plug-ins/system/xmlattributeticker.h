/* $Id: xmlattributeticker.h,v 1.1.2.1 2007/06/26 07:21:39 rufina Exp $
 *
 * ruffina, 2004
 * based on XMLAttributeTicker by NoFate, 2001
 */

#ifndef XMLATTRIBUTETICKER_H
#define XMLATTRIBUTETICKER_H

#include "scheduledxmlattribute.h"
#include "xmlinteger.h"
#include "xmlvariablecontainer.h"

class XMLAttributeTimer : public virtual XMLAttribute {
public:
	typedef ::Pointer<XMLAttributeTimer> Pointer;
	
	virtual ~XMLAttributeTimer( );

	virtual int getTime( ) const = 0;
	virtual void setTime( int ) = 0; 
};

class XMLAttributeTicker : public XMLVariableContainer {
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeTicker> Pointer;

	XMLAttributeTicker( );
	XMLAttributeTicker( int );
	virtual ~XMLAttributeTicker( );
	
	virtual void start( PCMemoryInterface* ) const = 0;
	virtual void end( PCMemoryInterface* ) const = 0;

	bool tick( PCMemoryInterface * );

	virtual int getTime( ) const;
	virtual void setTime( int );
	DLString getTimeString( bool ) const;
	DLString getUntilString( bool ) const;

protected:
	XML_VARIABLE XMLInteger time;
	XML_VARIABLE XMLInteger since;
};

class XMLAttributeOnlineTicker : public ScheduledXMLAttribute,  
                                 public XMLAttributeTicker
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeOnlineTicker> Pointer;

	virtual bool pull( PCharacter * );
};

class XMLAttributeMemoryTicker : public ScheduledPCMemoryXMLAttribute,  
				 public XMLAttributeTicker
{
XML_OBJECT
public:
	typedef ::Pointer<XMLAttributeMemoryTicker> Pointer;

	virtual bool pull( PCMemoryInterface * );
};



#endif

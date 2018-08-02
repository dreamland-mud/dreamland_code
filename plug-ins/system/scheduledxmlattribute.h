/* $Id: scheduledxmlattribute.h,v 1.1.2.5.24.1 2007/05/02 02:28:44 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef SCHEDULEDXMLATTRIBUTE_H
#define SCHEDULEDXMLATTRIBUTE_H

#include "xmlattribute.h"

class PCharacter;
class PCMemoryInterface;

class ScheduledXMLAttribute : public virtual XMLAttribute {
public:
	typedef ::Pointer<ScheduledXMLAttribute> Pointer;
	
	virtual ~ScheduledXMLAttribute( );
	virtual bool pull( PCharacter* ) = 0;
};

class ScheduledPCMemoryXMLAttribute : public virtual XMLAttribute {
public:
	typedef ::Pointer<ScheduledPCMemoryXMLAttribute> Pointer;
	
	virtual ~ScheduledPCMemoryXMLAttribute( );
	virtual bool pull( PCMemoryInterface * ) = 0;
};

#endif

/* $Id: objects.h,v 1.1.2.1.6.1 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef GANG_OBJECTS_H
#define GANG_OBJECTS_H

#include "xmlboolean.h"
#include "objectbehavior.h"

class Room;

/*
 * key
 */
class GangKey : public ObjectBehavior {
XML_OBJECT    
public:
	typedef ::Pointer<GangKey> Pointer;
	
	virtual void get( Character * );
	virtual bool extract( bool );
	virtual bool canSteal( Character * );

	XML_VARIABLE XMLBoolean needsReset;
};

/*
 * portal
 */

class GangPortal : public ObjectBehavior {
XML_OBJECT
public:
	typedef ::Pointer<GangPortal> Pointer;
    
        static bool canDrop( Room * );

};


#endif


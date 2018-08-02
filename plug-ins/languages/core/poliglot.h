/* $Id: poliglot.h,v 1.1.2.4 2008/04/04 20:04:56 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef POLIGLOT_H
#define POLIGLOT_H

#include "wanderer.h"
#include "basicmobilebehavior.h"

class Poliglot : public Wanderer, public BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<Poliglot> Pointer;
	
	Poliglot( );

	virtual void speech( Character *victim, const char *speech );
	virtual int  getOccupation( );

protected:
	virtual bool specIdle( );
	virtual bool handleMoveResult( Road &, int );
	virtual bool isHomesick( );
};

#endif


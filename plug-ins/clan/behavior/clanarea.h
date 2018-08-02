/* $Id: clanarea.h,v 1.1.2.2 2007/09/15 09:24:10 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CLANAREA_H
#define CLANAREA_H

#include "xmlinteger.h"
#include "areabehavior.h"
#include "clanreference.h"

class PCharacter;

class ClanArea : public AreaBehavior {
XML_OBJECT
public:
	typedef ::Pointer<ClanArea> Pointer;
    
	ClanArea( );

	virtual void update( );

	void createAltar( );
	ClanReference &getClan( );
	Object * findInvitation( PCharacter * );
	
	XML_VARIABLE XMLInteger itemVnum;
	XML_VARIABLE XMLInteger altarVnum;
	XML_VARIABLE XMLInteger roomVnum;
	XML_VARIABLE XMLInteger invitationVnum;
	XML_VARIABLE XMLInteger keyVnum;
	XML_VARIABLE XMLInteger bookVnum;

protected:
	XML_VARIABLE XMLClanReference clan;
};

#endif

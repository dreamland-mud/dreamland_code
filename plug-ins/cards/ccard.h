/* $Id: ccard.h,v 1.1.2.4.18.2 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CCARD_H
#define CCARD_H

#include "admincommand.h"

class PCharacter;

class CCard : public AdminCommand {
XML_OBJECT
public:
	typedef ::Pointer<CCard> Pointer;
    
	CCard( );

	virtual void run( Character *, const DLString & );
	
private:
	void doMob( PCharacter *, DLString& );
	void doChar( PCharacter *, DLString& );
	void doList( PCharacter *, DLString& );
	void usage( PCharacter * );

	static const DLString COMMAND_NAME;
};

#endif


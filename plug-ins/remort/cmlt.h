/* $Id: cmlt.h,v 1.1.2.3.4.2 2007/09/23 00:06:52 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef CMLT_H
#define CMLT_H

#include "commandplugin.h"
#include "defaultcommand.h"

class PCMemoryInterface;
class PCharacter;

class CMlt : public CommandPlugin, public DefaultCommand {
public:
	typedef ::Pointer<CMlt> Pointer;
    
	CMlt( );

	virtual void run( Character*, const DLString& constArguments );
	
private:
	void doCount( Character *, int );
	void doLimit( Character * );
	void doShowOther( Character *, PCMemoryInterface * );
	void doShowSelf( PCharacter * );

	static const DLString COMMAND_NAME;
		
};

#endif


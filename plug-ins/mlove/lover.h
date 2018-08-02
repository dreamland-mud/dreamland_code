/* $Id: lover.h,v 1.1.2.4.6.1 2007/06/26 07:18:03 rufina Exp $
 * ruffina, 2003
 */

#ifndef LOVER_H
#define LOVER_H

#include "commandplugin.h"
#include "defaultcommand.h"


class Lover : public CommandPlugin, public DefaultCommand
{
public:
	typedef ::Pointer<Lover> Pointer;

public:
	Lover( );
	
	virtual void run( Character*, const DLString& constArguments );
	
private:
	void list( Character *, DLString );
	void del( Character *, DLString );
	void add( Character *, DLString );
	void usage( Character * );    
	
	static const DLString COMMAND_NAME;
	static const DLString XMLAttributeLoverString;
};




#endif

/* $Id: wrappersplugin.h,v 1.1.4.2.26.1 2009/11/04 03:24:33 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef WRAPPERSPLUGIN_H
#define WRAPPERSPLUGIN_H

#include "plugin.h"
#include "xmlvariablecontainer.h"

class WrappersPlugin : public Plugin, public XMLVariableContainer {
XML_OBJECT
public:	
	typedef ::Pointer<WrappersPlugin> Pointer;
	
	virtual void initialization( );
	virtual void destruction( );
	
	static void linkTargets();
};

#endif

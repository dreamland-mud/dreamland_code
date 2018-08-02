/* $Id: descriptorstatelistener.h,v 1.1.4.2 2005/09/07 19:49:31 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef DESCRIPTORSTATELISTENER_H
#define DESCRIPTORSTATELISTENER_H

#include "plugin.h"

class Descriptor;

class DescriptorStateListener : public virtual Plugin {
public:
	typedef ::Pointer<DescriptorStateListener> Pointer;

	virtual void initialization( );
	virtual void destruction( );
	
	virtual void run ( int oldState, int newState, Descriptor *d ) = 0;
};

#endif

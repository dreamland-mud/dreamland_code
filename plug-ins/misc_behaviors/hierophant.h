/* $Id: hierophant.h,v 1.1.2.7 2005/11/26 16:59:51 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef HIEROPHANT_H
#define HIEROPHANT_H

#include "basicmobilebehavior.h"

class Hierophant : public BasicMobileDestiny {
XML_OBJECT
public:
	typedef ::Pointer<Hierophant> Pointer;
    
	Hierophant( );

	virtual void speech( Character *victim, const char *speech );
	virtual void tell ( Character *victim, const char *speech );
};

#endif




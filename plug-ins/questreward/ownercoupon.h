/* $Id: ownercoupon.h,v 1.1.2.1 2007/05/02 02:32:37 rufina Exp $
 *
 * ruffina, 2007
 */

#ifndef OWNERCOUPON_H
#define OWNERCOUPON_H

#include "objectbehavior.h"

class OwnerCoupon : public ObjectBehavior {
XML_OBJECT
public:
	typedef ::Pointer<OwnerCoupon> Pointer;
    
	virtual bool use( Character *, const char *);
        virtual bool hasTrigger( const DLString &  );
};

#endif


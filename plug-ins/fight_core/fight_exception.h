/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __FIGHT_EXCEPTION_H__
#define __FIGHT_EXCEPTION_H__

#include "exception.h"

struct VictimDeathException : public Exception {
    virtual ~VictimDeathException( ) throw( );
};



#endif

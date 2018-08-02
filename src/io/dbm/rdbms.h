/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#ifndef __RDBMS_H__
#define __RDBMS_H__

#include "config.h"

#ifdef HAS_BDB 
    #include "rdbms_bdb.h"
#else
    #error 
#endif


extern template class DbContext<unsigned long int>;
extern template class DbContext<unsigned long long>;
extern template class DbContext<DLString>;

#endif

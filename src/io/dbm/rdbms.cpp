/* $Id$
 *
 * ruffina, Dream Land, 2005
 */
/* $Id$
 * 
 * unicorn, Forgotten Dungeon, 2005
 */

#include "config.h"

#ifdef HAS_BDB 
    #include "rdbms_bdb.cpp"
#else
    #error
#endif


template class DbContext<unsigned long int>;
template class DbContext<unsigned long long>;
template class DbContext<DLString>;

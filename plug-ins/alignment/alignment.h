/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ALIGNMENT_H__
#define __ALIGNMENT_H__

#include <sstream>
#include "dlstring.h"

class Character;
class PCharacter;

struct alignment_t {
    int minValue;
    int aveValue;
    int maxValue;
    const char *rname;
};

extern const struct alignment_t alignment_table [];

DLString align_name_for_range( int min, int max );
int      align_choose_allowed( PCharacter *, int n );
int      align_choose_allowed( PCharacter *, const DLString& );
void     align_print_allowed( PCharacter *, ostringstream & );
DLString align_name( Character * );
DLString align_max( PCharacter * );
DLString align_min( PCharacter * );

#define ALIGN_ERROR 0xffff

#endif

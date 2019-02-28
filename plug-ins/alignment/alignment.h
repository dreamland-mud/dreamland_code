/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ALIGNMENT_H__
#define __ALIGNMENT_H__

#include <sstream>
#include "dlstring.h"
#include "grammar_entities.h"

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
const char *align_name_short(Character *ch, const Grammar::MultiGender &g);

#define ALIGN_ERROR 0xffff

#define ALIGN_IS_GOOD(a)           ((a) >= 350)
#define ALIGN_IS_EVIL(a)           ((a) <= -350)
#define ALIGN_IS_NEUTRAL(a)        (!ALIGN_IS_GOOD(a) && !ALIGN_IS_EVIL(a))
#define ALIGN_NUMBER(a)            (ALIGN_IS_GOOD(a) ? N_ALIGN_GOOD : ALIGN_IS_EVIL(a) ? N_ALIGN_EVIL : N_ALIGN_NEUTRAL)
#endif

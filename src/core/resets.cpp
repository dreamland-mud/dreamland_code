#include "resets.h"
#include "flags-impl.h"
#include "enumeration-impl.h"
#include "autoflags.h"
#include "def.h"

reset_data::reset_data()
    : command('X'), 
      arg1(0), arg2(0), arg3(0), arg4(0),
      flags(0, &reset_flags), 
      rand(RAND_NONE, &rand_table),
      bestTier(0)
{

}      



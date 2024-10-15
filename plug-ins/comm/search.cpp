#include "commandtemplate.h"
#include "character.h"
#include "interp.h"
#include "arg_utils.h"

CMDRUNP( search )
{
    DLString args = argument, arg = args.getOneArgument( );

    if (!arg.empty( )) {
        if (arg_is(arg, "stones")) {
            interpret_cmd( ch, "searchstones", args.c_str( ) );
            return;
        }
    }

    ch->pecho("Ты можешь искать только камни.");
}


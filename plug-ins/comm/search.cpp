#include "commandtemplate.h"
#include "character.h"
#include "interp.h"

CMDRUNP( search )
{
    DLString args = argument, arg = args.getOneArgument( );

    if (!arg.empty( )) {
        if (arg.strPrefix( "stones" ) || arg.strPrefix( "камни" )) {
            interpret_cmd( ch, "searchstones", args.c_str( ) );
            return;
        }
    }

    ch->pecho("Ты можешь искать только камни.");
}


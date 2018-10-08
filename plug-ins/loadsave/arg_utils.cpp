/* $Id$
 *
 * ruffina, 2004
 */
#include "dlstring.h"
#include "dl_strings.h"
#include "arg_utils.h"

/*--------------------------------------------------------------------------
 * command arguments parsing 
 *--------------------------------------------------------------------------*/
bool arg_contains_someof( const DLString &arg, const char *namesList )
{
    DLString names = namesList, n;
    
    while (!( n = names.getOneArgument( ) ).empty( )) 
        if (is_name( n.c_str( ), arg.c_str( ) ))
            return true;
    
    return false;
}

bool arg_oneof_strict( const DLString &arg, const char *var1, const char *var2, const char *var3, const char *var4 )
{
    if (arg.empty( ))
        return false;

    if (var1 && arg ^ var1)
        return true;

    if (var2 && arg ^ var2)
        return true;

    if (var3 && arg ^ var3)
        return true;

    if (var4 && arg ^ var4)
        return true;

    return false;
}

bool arg_has_oneof( const DLString &arg, const char *var1, const char *var2, const char *var3, const char *var4 )
{
    if (arg.empty( ))
        return false;

    if (var1 && is_name( var1, arg.c_str( ) ))
        return true;

    if (var2 && is_name( var2, arg.c_str( ) ))
        return true;

    if (var3 && is_name( var3, arg.c_str( ) ))
        return true;

    if (var3 && is_name( var3, arg.c_str( ) ))
        return true;

    return false;
}

bool arg_oneof( const DLString &arg, const char *var1, const char *var2, const char *var3, const char *var4 )
{
    if (arg.empty( ))
        return false;

    if (var1 && arg.strPrefix( var1 ))
        return true;

    if (var2 && arg.strPrefix( var2 ))
        return true;

    if (var3 && arg.strPrefix( var3 ))
        return true;

    if (var4 && arg.strPrefix( var4 ))
        return true;

    return false;
}

bool arg_is_help( const DLString &arg )
{
    return arg_oneof( arg, "help", "?", "помощь", "справка" );
}

bool arg_is_list( const DLString &arg )
{
    return arg_oneof( arg, "list", "список" );
}

bool arg_is_info( const DLString &arg )
{
    return arg_oneof( arg, "info", "информация" );
}

bool arg_is_time( const DLString &arg )
{
    return arg_oneof( arg, "time", "время" );
}

bool arg_is_pk( const DLString &arg )
{
    return arg_oneof_strict( arg, "pk", "пк" );
}

bool arg_is_show( const DLString &arg )
{
    return arg_oneof( arg, "show", "показ", "показать" );
}

bool arg_is_in( const DLString &arg )
{
    return arg_oneof_strict( arg, "i", "in", "в" );
}

bool arg_is_on( const DLString &arg )
{
    return arg_oneof_strict( arg, "on", "на" );
}

bool arg_is_from( const DLString &arg )
{
    return arg_oneof_strict( arg, "from", "из", "с", "со" );
}

bool arg_is_to( const DLString &arg )
{
    return arg_oneof_strict( arg, "to", "на" );
}

bool arg_is_yes( const DLString &arg )
{
    return arg_oneof_strict( arg, "yes", "y", "да", "д" );
}

bool arg_is_no( const DLString &arg )
{
    return arg_oneof_strict( arg, "no", "n", "нет", "н" );
}

bool arg_is_switch_off( const DLString &arg )
{
    return arg_oneof_strict( arg, "off", "офф", "выкл" );
}

bool arg_is_switch_on( const DLString &arg )
{
    return arg_oneof_strict( arg, "on", "вкл" );
}

bool arg_is_self( const DLString &arg )
{
    return arg_oneof_strict( arg, "self", "я", "себе", "себя" );
}

bool arg_is_ugly( const DLString &arg )
{
    return arg_oneof_strict( arg, "ugly", "vampire", "вампир" );
}

bool arg_is_silver( const DLString &arg )
{
    return arg_oneof_strict( arg, "coin", "coins", "silver" )
           || arg_oneof_strict( arg, "серебро", "серебра", "монет" );
}

bool arg_is_gold( const DLString &arg )
{
    return arg_oneof_strict( arg, "gold", "золото", "золота", "золотых" );
}

bool arg_is_alldot( const DLString &arg )
{
    return arg == "all" 
           || arg == "все" 
           || arg == "всё" 
           || !str_prefix( "all.", arg.c_str( ) )
           || !str_prefix( "все.", arg.c_str( ) )
           || !str_prefix( "всё.", arg.c_str( ) );
}

bool arg_is_all( const DLString &arg )
{
    return arg == "all" || arg == "все" || arg == "всё"; 
}



/* $Id$
 *
 * ruffina, 2004
 */
#include "dlstring.h"
#include "dl_strings.h"
#include "jsoncpp/json/value.h"
#include "arg_utils.h"
#include "configurable.h"


Json::Value synonyms;
CONFIGURABLE_LOADED(grammar, synonyms)
{
    synonyms = value;
}


/*--------------------------------------------------------------------------
 * command arguments parsing 
 *--------------------------------------------------------------------------*/

// Synonyms lookup by keyword. True if for keyword "help" arg is one of "?", "help", "h", "довідка" etc.
bool arg_is(const DLString &arg, const DLString &keyword)
{
    if (arg.empty())
        return false;

    if (arg.strPrefix(keyword))
        return true;

    if (!synonyms.isMember(keyword))
        return false;

    for (auto &synon: synonyms[keyword]) {
        if (arg.strPrefix(synon.asString()))
            return true;
    }

    return false;
}

// Exact synonym lookup.
bool arg_is_strict(const DLString &arg, const DLString &keyword)
{
    if (arg.empty())
        return false;

    if (arg == keyword)
        return true;

    if (!synonyms.isMember(keyword))
        return false;

    for (auto &synon: synonyms[keyword]) {
        if (arg == synon.asString())
            return true;
    }

    return false;
}

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
    return arg_is(arg, "help");
}

bool arg_is_list( const DLString &arg )
{
    return arg_is(arg, "list");
}

bool arg_is_info( const DLString &arg )
{
    return arg_is(arg, "info");
}

bool arg_is_time( const DLString &arg )
{
    return arg_is(arg, "time");
}

bool arg_is_pk( const DLString &arg )
{
    return arg_is_strict(arg, "pk");
}

bool arg_is_show( const DLString &arg )
{
    return arg_is(arg, "show");
}

bool arg_is_in( const DLString &arg )
{
    return arg_is_strict(arg, "in");
}

bool arg_is_on( const DLString &arg )
{
    return arg_is_strict(arg, "on");
}

bool arg_is_from( const DLString &arg )
{
    return arg_is_strict(arg, "from");
}

bool arg_is_to( const DLString &arg )
{
    return arg_is_strict(arg, "to");
}

bool arg_is_yes( const DLString &arg )
{
    return arg_is_strict(arg, "yes");
}

bool arg_is_no( const DLString &arg )
{
    return arg_is_strict(arg, "no");
}

bool arg_is_switch_off( const DLString &arg )
{
    return arg_is_strict(arg, "switchoff");
}

bool arg_is_switch_on( const DLString &arg )
{
    return arg_is_strict(arg, "switchon");
}

bool arg_is_self( const DLString &arg )
{
     return arg_is_strict(arg, "self");
}

bool arg_is_ugly( const DLString &arg )
{
    return arg_is(arg, "vampire");
}

bool arg_is_silver( const DLString &arg )
{
    return arg_is_strict(arg, "silver");
}

bool arg_is_gold( const DLString &arg )
{
    return arg_is_strict(arg, "gold");
}

bool arg_is_copy( const DLString &arg )
{
    return arg_is(arg, "copy");
}

bool arg_is_paste( const DLString &arg )
{
    return arg_is(arg, "paste");
}

bool arg_is_web( const DLString &arg )
{
    return arg_is_strict(arg, "web");
}

bool arg_is_money( const DLString &arg )
{
    return arg_is(arg, "silver") || arg_is(arg, "gold");
}

bool arg_is_alldot( const DLString &arg )
{
    if (arg_is_all(arg))
        return true;

    list<DLString> splitByDot = arg.split(".");
    if (splitByDot.size() == 1)
        return false;
    
    DLString argBeforeDot = splitByDot.front();
    return arg_is_all(argBeforeDot);
}

bool arg_is_all( const DLString &arg )
{
    return arg_is_strict(arg, "all");
}

bool arg_is_clear( const DLString &arg )
{
    return arg_is(arg, "clear");
}

/** Remove surrounding quotes from an argument. */
DLString arg_unquote(const DLString &arg)
{
    return DLString(arg).substitute('\'', ' ').substitute('\"', ' ').stripWhiteSpace();
}

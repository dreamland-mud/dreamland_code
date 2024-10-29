/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ARG_UTILS_H
#define ARG_UTILS_H

class DLString;

bool arg_is(const DLString &arg, const DLString &keyword);
bool arg_is_strict(const DLString &arg, const DLString &keyword);


bool        arg_contains_someof( const DLString &arg, const char *namesList );
bool        arg_oneof( const DLString &arg, const char *var1 = NULL, const char *var2 = NULL, const char *var3 = NULL, const char *var4 = NULL );
bool        arg_oneof_strict( const DLString &arg, const char *var1 = NULL, const char *var2 = NULL, const char *var3 = NULL, const char *var4 = NULL );
bool        arg_has_oneof( const DLString &arg, const char *var1 = NULL, const char *var2 = NULL, const char *var3 = NULL, const char *var4 = NULL );
bool        arg_is_help( const DLString &arg );
bool        arg_is_list( const DLString &arg );
bool        arg_is_info( const DLString &arg );
bool        arg_is_show( const DLString &arg );
bool        arg_is_time( const DLString &arg );
bool        arg_is_pk( const DLString &arg );
bool        arg_is_in( const DLString &arg );
bool        arg_is_on( const DLString &arg );
bool        arg_is_yes( const DLString &arg );
bool        arg_is_no( const DLString &arg );
bool        arg_is_from( const DLString &arg );
bool        arg_is_to( const DLString &arg );
bool        arg_is_switch_on( const DLString &arg );
bool        arg_is_switch_off( const DLString &arg );
bool        arg_is_copy( const DLString &arg );
bool        arg_is_paste( const DLString &arg );
bool        arg_is_web( const DLString &arg );
bool        arg_is_self( const DLString &arg );
bool        arg_is_ugly( const DLString &arg );
bool        arg_is_silver( const DLString &arg );
bool        arg_is_gold( const DLString &arg );
bool        arg_is_money( const DLString &arg );
bool        arg_is_alldot( const DLString &arg );
bool        arg_is_all( const DLString &arg );
bool        arg_is_clear( const DLString &arg );
bool        arg_is_lang(const DLString &arg);
DLString arg_unquote(const DLString &arg);

#endif


/* $Id: dl_strings.h,v 1.1.2.4 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef DL_STRINGS_H
#define DL_STRINGS_H

class DLString;

int     str_cmp( const char *astr, const char *bstr );
int     strn_cmp(const char *arg1, const char *arg2, int n);
bool    str_not_equal( const char *astr, const char *bstr );
bool        str_prefix        ( const char *astr, const char *bstr );
bool        is_name( const char *arg1, const char *arg2 );
DLString russian_case( const DLString & description, char gram_case );
bool        is_number        ( const char *arg );
bool    is_positive_number( const char *arg );
int        number_argument        ( char *argument, char *arg );
int        mult_argument        ( char *argument, char *arg );
char *        one_argument        ( char *argument, char *arg_first );
char *  str_str(char *cs, const char *ct);

#endif

/* $Id: dl_strings.h,v 1.1.2.4 2009/11/02 13:48:11 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef DL_STRINGS_H
#define DL_STRINGS_H

#include <list>

class DLString;

bool        str_prefix        ( const char *astr, const char *bstr );
bool        is_name( const char *arg1, const char *arg2 );

/** Return a particular grammar case of a string. */
DLString russian_case( const DLString & description, char gram_case );
/** Return a string of all space-separated grammar cases, colours stripped off. */
DLString russian_case_all_forms(const DLString &string);
std::list<DLString> russian_cases(const DLString &str);

bool        is_number        ( const char *arg );
int        number_argument        ( char *argument, char *arg );
int        mult_argument        ( char *argument, char *arg );
char *        one_argument        ( char *argument, char *arg_first );

#endif

/* $Id$
 *
 * ruffina, 2004
 */
#ifndef FREAD_UTILS_H
#define FREAD_UTILS_H

#include "dlstring.h"

char        fread_letter        ( FILE *fp );
int        fread_number        ( FILE *fp );
long long fread_number64( FILE *fp );
long         fread_flag        ( FILE *fp );
char *        fread_string        ( FILE *fp );
char *  fread_string_eol ( FILE *fp );
void        fread_to_eol        ( FILE *fp );
DLString fread_dlstring_to_eol( FILE *fp );
DLString fread_dlstring( FILE *fp );
char *        fread_word        ( FILE *fp );
long        flag_convert        ( char letter);

#endif

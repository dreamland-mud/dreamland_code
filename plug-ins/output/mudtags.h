/* $Id$
 *
 * ruffina, 2004
 */
#ifndef MUDTAGS_H
#define MUDTAGS_H

#include <sstream>

class Character;

void mudtags_convert( const char *text, ostringstream &out, Character *ch = NULL );
void mudtags_convert_nocolor( const char *text, ostringstream &out, Character *ch = NULL );
void mudtags_raw( const char *text, ostringstream &out );
void vistags_convert( const char *text, ostringstream &out, Character *ch = NULL );

#endif


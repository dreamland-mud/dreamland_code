/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __COMM_H__
#define __COMM_H__
#include "dlstring.h"


class Descriptor;
class Character;

char *get_multi_command( Descriptor *d, char *argument);
bool read_from_descriptor( Descriptor *d );
bool read_from_buffer( Descriptor *d );
bool process_output( Descriptor *d, bool fPrompt );
void page_to_char( const char *txt, Character *ch );

void do_help( Descriptor *d, const char *topic, bool fColor );
void do_help( Character *ch, const char *topic );

Descriptor * descriptor_find_named( Descriptor *myD, const DLString &myName, int state = -1 );

string create_nonce(int len);

#endif

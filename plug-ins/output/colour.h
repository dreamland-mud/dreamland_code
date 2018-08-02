/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __COLOUR_H__
#define __COLOUR_H__

#include <sstream>

using namespace std;

/*
 * Colour stuff by Lope of Loping Through The MUD
 */
#define CLEAR		"\033[0m"	/* Resets Colour	*/
#define C_BLINK		"\033[0;5m"	/* Blink Colours	*/
#define C_RED		"\033[0;31m"	/* Normal Colours	*/
#define C_GREEN		"\033[0;32m"
#define C_YELLOW	"\033[0;33m"
#define C_BLUE		"\033[0;34m"
#define C_MAGENTA	"\033[0;35m"
#define C_CYAN		"\033[0;36m"
#define C_WHITE		"\033[0;37m"
#define C_D_GREY	"\033[1;30m"  	/* Light Colors		*/
#define C_B_RED		"\033[1;31m"
#define C_B_GREEN	"\033[1;32m"
#define C_B_YELLOW	"\033[1;33m"
#define C_B_BLUE	"\033[1;34m"
#define C_B_MAGENTA	"\033[1;35m"
#define C_B_CYAN	"\033[1;36m"
#define C_B_WHITE	"\033[1;37m"
#define CLR_PUSH    01
#define CLR_POP	    02

#define ANSI_CLEARSCR       "\033[2J"
#define ANSI_HOME           "\033[0;0H"
#define ANSI_COLOR_RESET    "\033[0;37;40m"
#endif

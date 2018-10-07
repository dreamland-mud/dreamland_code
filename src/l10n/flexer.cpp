/* $Id: flexer.cpp,v 1.1.2.4 2010-09-01 21:20:47 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <sstream>
#include "flexer.h"
#include "dl_ctype.h"

using std::basic_ostringstream;
using namespace Grammar;

const char Flexer::FLEX_DELIMITER = '|';

bool Flexer::isWordDelimiter(char c)
{
    return dl_isspace(c) || c == '-';    
}

bool Flexer::isFlexDelimiter(char c)
{
    return c == FLEX_DELIMITER;
}

DLString Flexer::getRoot(const DLString &str) 
{
    return flexAux(str, 0, true, false);
}

DLString Flexer::getFlexion(const DLString &str, int part_num) 
{
    return flexAux(str, part_num, false, true);
}

DLString Flexer::flex(const DLString &str, int part_num) 
{
    return flexAux(str, part_num, true, true);
}

DLString Flexer::flexAux(const DLString &str, int part_num, bool fNeedRoot, bool fNeedEnding) 
{
    int case_counter, iPhase;
    register char tempchar;
    basic_ostringstream<char> buf;
    
    case_counter = part_num;
    iPhase = PHASE_COPYTOBUF;

    for (size_t cnt = 0; cnt < str.size(); cnt++) {
	tempchar = str.at(cnt); 
	
	if (isWordDelimiter(tempchar))
	    iPhase=PHASE_COPYTOBUF; /* reset */

	if (iPhase != PHASE_FINDNEED)
	    case_counter = part_num;

	if (isFlexDelimiter(tempchar)) {
	    if (iPhase == PHASE_COPYEND)
		iPhase=PHASE_FINDNEXT;

	    if (iPhase == PHASE_COPYTOBUF)
		iPhase=PHASE_FINDNEED;

	    if (iPhase == PHASE_FINDNEED) {
		case_counter--;
		if (case_counter <= 0)
		    iPhase = PHASE_COPYEND;
	    }
	}
	else if ((iPhase == PHASE_COPYEND && fNeedEnding)
	         || (iPhase == PHASE_COPYTOBUF && fNeedRoot)) 
	{
	    buf << tempchar;
	}
    }

    return buf.str();
}


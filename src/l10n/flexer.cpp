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
    bool hasColour = false;
    
    case_counter = part_num;
    iPhase = PHASE_COPYTOBUF;

    for (size_t cnt = 0; cnt < str.size(); cnt++) {
        tempchar = str.at(cnt); 
        
        if (isWordDelimiter(tempchar))
            iPhase=PHASE_COPYTOBUF; /* reset */

        if (iPhase != PHASE_FINDNEED)
            case_counter = part_num;

        // Quick (and unprecise) check if the string contains colors.
        if (tempchar == '{')
            hasColour = true;

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

    DLString result = buf.str();

    if (!hasColour)
        return result;

    // Fence the noun in push/pop colour instead of closing it with {x: {x is a
    // total reset in mudtags and would also end an enclosing {l/{S section, so a
    // coloured name inside a localized message leaked the other languages. Inner
    // resets become pop+push for the same reason; one that closed a {h link
    // still closes it, as {hx. The push goes right before the first code, not
    // at index 0, so plain leading text stays first for upperFirstCharacter and
    // first-word matching; that text prints in the enclosing colour either way.
    size_t first = result.find( '{' );
    if (first == DLString::npos)
        return result;

    DLString fenced( result.substr( 0, first ) );
    fenced << "{1";
    bool inLink = false;
    for (size_t i = first; i < result.size(); i++) {
        char c = result.at(i);
        if (c == '{' && i + 1 < result.size()) {
            char n = result.at(i + 1);
            if (n == '{') {
                fenced << "{{";
                i++;
                continue;
            }
            if (n == 'x' || n == 'X') {
                if (inLink)
                    fenced << "{hx";
                inLink = false;
                fenced << "{2{1";
                i++;
                continue;
            }
            if (n == 'h' && i + 2 < result.size())
                inLink = (result.at(i + 2) != 'x' && result.at(i + 2) != 'X');
        }
        fenced << c;
    }
    if (inLink)
        fenced << "{hx";
    fenced << "{2";

    return fenced;
}


/* $Id: flexer.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_FLEXER_H
#define L10N_FLEXER_H

#include "dlstring.h"

namespace Grammar {

class Flexer {
public:    

    static DLString getRoot(const DLString &str);
    static DLString getFlexion(const DLString &str, int num);
    static DLString flex(const DLString &str, int num);
    
    static bool isWordDelimiter(char c); 
    static bool isFlexDelimiter(char c); 

private:
    static const char FLEX_DELIMITER;
    enum {
        PHASE_NONE,
        PHASE_COPYTOBUF,
        PHASE_FINDNEED,        
        PHASE_COPYEND,        
        PHASE_FINDNEXT,        
    };
    static DLString flexAux(const DLString &str, int part_num, bool fNeedRoot, bool fNeedEnding);
};

}

#endif

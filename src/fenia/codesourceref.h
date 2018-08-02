/* $Id: codesourceref.h,v 1.1.4.1.6.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: codesourceref.h,v 1.1.4.1.6.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */
#ifndef __CODESOURCEREF_H__
#define __CODESOURCEREF_H__

#include <pointer.h>
#include <xmlvariable.h>
#include <xmlnode.h>

namespace Scripting {
    
class CodeSource;

class CodeSourceRef {
public:
    CodeSourceRef();
    ~CodeSourceRef();

    int line;
    ::Pointer<CodeSource> source;
};

ostream &operator << (ostream &os, const CodeSourceRef &csr);

}
#endif

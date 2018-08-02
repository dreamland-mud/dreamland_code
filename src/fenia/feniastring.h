/* $Id: feniastring.h,v 1.1.2.2.18.2 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: feniastring.h,v 1.1.2.2.18.2 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __FENIASTRING_H__
#define __FENIASTRING_H__

#include <dlstring.h>

#include "native.h"

namespace Scripting {

class FeniaString : public DLString {
NMI_OBJECT
public:
    typedef NativeTraits<FeniaString> Traits;

    FeniaString() { }
    FeniaString(const DLString &dls) : DLString(dls) { }
};

}

#endif /* __FENIASTRING_H__ */

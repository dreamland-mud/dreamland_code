/* $Id: flow.h,v 1.1.4.2.18.1 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: flow.h,v 1.1.4.2.18.1 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __FLOW_H__
#define __FLOW_H__

#include "register-decl.h"

namespace Scripting {

struct FlowCtl {
    static const int NEXT = 1;
    static const int BREAK = 2;
    static const int CONTINUE = 3;
    static const int RETURN = 4;

    FlowCtl(int t) : type(t) { }
    FlowCtl(int t, Register r) : type(t), ret(r) { }

    int type;
    Register ret;
};

}

#endif

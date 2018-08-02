/* $Id: phase.h,v 1.1.4.2.18.1 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: phase.h,v 1.1.4.2.18.1 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */
#ifndef __PHASE_H__
#define __PHASE_H__

namespace Scripting {

class Function;
class Object;

class DereferenceListener {
public:
    virtual void notify(Function *) = 0;
    virtual void notify(Object *) = 0;

    static DereferenceListener *instance;
};

extern bool gc;
}

#endif

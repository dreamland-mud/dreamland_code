/* $Id: nounholder.h,v 1.1.2.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_NOUNHOLDER_H
#define L10N_NOUNHOLDER_H

#include "pointer.h"
#include "dlobject.h"

namespace Grammar {

class Noun;

class NounHolder : public virtual DLObject {
public:
    typedef ::Pointer<Noun> NounPointer;

    virtual ~NounHolder();
    virtual NounPointer toNoun(const DLObject *forWhom = 0, int flags = 0) const = 0;
};

}

#endif

/* $Id: noun.h,v 1.1.2.5 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_NOUN_H
#define L10N_NOUN_H

#include "dlstring.h"
#include "dlobject.h"
#include "grammar_entities.h"

namespace Grammar {
    
class Noun : public virtual DLObject {
public:
    typedef ::Pointer<Noun> Pointer;

    virtual ~Noun();
    
    virtual Gender getGender() const = 0;
    virtual Number getNumber() const = 0;
    virtual DLString decline(const Case &c) const;
    virtual const DLString &getFullForm() const = 0;
    
    DLString normal() const;
    MultiGender getMultiGender() const;
};

}

#endif

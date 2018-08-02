/* $Id: adjective.h,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_ADJECTIVE_H
#define L10N_ADJECTIVE_H

#include "dlstring.h"
#include "grammar_entities.h"

namespace Grammar {
   
class Noun;

class Adjective {
public:
    virtual ~Adjective();
    
    virtual const DLString & getFullForm() const = 0;

    DLString decline(const Noun &who, const Case &c) const;
    DLString decline(const Case &c, const MultiGender &mg = MultiGender::MASCULINE) const;
};

}

#endif

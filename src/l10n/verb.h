/* $Id: verb.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_VERB_H
#define L10N_VERB_H

#include "dlstring.h"
#include "grammar_entities.h"

namespace Grammar {
   
class Noun;

class Verb {
public:

    virtual ~Verb();
    
    virtual const DLString & getRoot() const = 0;
    virtual const DLString & getEndings() const = 0;

    DLString conjugate(const Noun &who) const;
    DLString conjugate(const MultiGender &mg) const;
};

}

#endif

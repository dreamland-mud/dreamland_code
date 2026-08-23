/* $Id: ru_pronouns.h,v 1.1.2.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef RU_PRONOUNS_H
#define RU_PRONOUNS_H

#include "pronouns.h"
#include "inflectedstring.h"
#include "lang.h"

namespace Grammar {

extern const IndefinitePronoun::AnimacyCases ru_indefinite_pronouns;
extern const PersonalPronoun::Persons ru_personal_pronouns;
extern const PosessivePronoun::PosessionGenders ru_posessive_pronouns;

extern const InflectedString::Pointer somebody;
extern const InflectedString::Pointer something;

// Invisible-actor placeholder for the viewer's language (see ru_pronouns.cpp).
const InflectedString::Pointer & getSomebody(lang_t lang);
const InflectedString::Pointer & getSomething(lang_t lang);

}

#endif

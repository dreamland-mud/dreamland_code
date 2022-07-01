/* $Id: noun.cpp,v 1.1.2.7 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "grammar_entities_impl.h"
#include "noun.h"
#include "flexer.h"

using namespace Grammar;

/*----------------------------------------------------------------------------
 * Noun
 *--------------------------------------------------------------------------*/
Noun::~Noun()
{
}

DLString Noun::normal() const
{
    return decline(Case::NOMINATIVE);
}

MultiGender Noun::getMultiGender() const
{
    return MultiGender(getGender(), getNumber());
}

DLString Noun::decline(const Case &c) const
{
    return Flexer::flex(getFullForm(), c + 1);
}


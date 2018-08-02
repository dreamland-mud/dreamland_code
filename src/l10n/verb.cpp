/* $Id: verb.cpp,v 1.1.2.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "grammar_entities_impl.h"
#include "verb.h"
#include "noun.h"
#include "flexer.h"

using namespace Grammar;

Verb::~Verb()
{
}

DLString Verb::conjugate(const Noun &who) const
{
    return conjugate(who.getMultiGender());
}

DLString Verb::conjugate(const MultiGender &mg) const 
{
    return getRoot() + Flexer::flex(getEndings(), mg + 1); 
}


/* $Id: adjective.cpp,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "grammar_entities_impl.h"
#include "adjective.h"
#include "noun.h"
#include "flexer.h"

using namespace Grammar;

Adjective::~Adjective()
{
}

DLString Adjective::decline(const Noun &who, const Case &c) const
{
    return decline(who.getMultiGender(), c);
}

DLString Adjective::decline(const Case &c, const MultiGender &mg) const
{
    int part_num = mg * CASE_MAX + c + 1;
    return Flexer::flex(getFullForm(), part_num);
}


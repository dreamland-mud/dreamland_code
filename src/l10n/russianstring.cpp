/* $Id: russianstring.cpp,v 1.1.2.7 2009/11/08 17:33:28 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "grammar_entities_impl.h"
#include "russianstring.h"

using namespace Grammar;

RussianString::RussianString()
{
    fillCachedForms();
}

RussianString::RussianString(const DLString &ff)
                  : fullForm(ff)
{
    fillCachedForms();
}

RussianString::RussianString(const DLString &ff, const MultiGender &mg)
                  : fullForm(ff), mg(mg)
{
    fillCachedForms();
}

Gender RussianString::getGender() const
{
    return mg.toGender();
}

Number RussianString::getNumber() const
{
    return mg.toNumber();
}

const DLString &RussianString::getFullForm() const
{
    return fullForm;
}

NounHolder::NounPointer RussianString::toNoun(const DLObject *, int) const
{
    RussianString::Pointer me(this);
    link();
    return me;
}

void RussianString::setFullForm(const DLString &ff)
{
    fullForm = ff;
    fillCachedForms();
}

void RussianString::setGender(const MultiGender &mg) 
{
    this->mg = mg;
}

void RussianString::fillCachedForms()
{
    cachedForms.clear();
    cachedForms.resize(Case::MAX + 1);
    cachedForms[Case::MAX] = "";

    for (int c = Case::NOMINATIVE; c < Case::MAX; c++) {
	cachedForms[c] = FlexedNoun::decline(Case(c));
	cachedForms[Case::MAX] << cachedForms[c] << " ";
    }
}

DLString RussianString::decline(const Case &c) const
{
    return cachedForms[c];
}


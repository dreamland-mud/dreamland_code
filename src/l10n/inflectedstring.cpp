/* $Id: inflectedstring.cpp,v 1.1.2.7 2009/11/08 17:33:28 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "grammar_entities_impl.h"
#include "inflectedstring.h"
#include "flexer.h"
#include "logstream.h"

using namespace Grammar;

InflectedString::InflectedString()
{
    fillCachedForms();
}

InflectedString::InflectedString(const DLString &ff)
                  : fullForm(ff)
{
    fillCachedForms();
}

InflectedString::InflectedString(const DLString &ff, const MultiGender &mg)
                  : fullForm(ff), mg(mg)
{
    fillCachedForms();
}

Gender InflectedString::getGender() const
{
    return mg.toGender();
}

Number InflectedString::getNumber() const
{
    return mg.toNumber();
}

const DLString &InflectedString::getFullForm() const
{
    return fullForm;
}

NounHolder::NounPointer InflectedString::toNoun(const DLObject *, int) const
{
    InflectedString::Pointer me(this);
    link();
    return me;
}

void InflectedString::setFullForm(const DLString &ff)
{
    fullForm = ff;
    fillCachedForms();
}

void InflectedString::setGender(const MultiGender &mg) 
{
    this->mg = mg;
}

void InflectedString::fillCachedForms()
{
    cachedForms.clear();
    cachedForms.resize(Case::MAX + 1);
    cachedForms[Case::MAX] = "";

    for (int c = Case::NOMINATIVE; c < Case::MAX; c++) {
        cachedForms[c] = Flexer::flex(fullForm, c + 1);        
        cachedForms[Case::MAX] << cachedForms[c] << " ";
    }
}

const DLString& InflectedString::decline(const Case &c) const
{
    return cachedForms[c];
}


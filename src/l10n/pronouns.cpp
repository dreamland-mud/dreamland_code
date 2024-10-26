/* $Id: pronouns.cpp,v 1.1.2.5 2010-09-01 21:20:47 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include "grammar_entities_impl.h"
#include "pronouns.h"
#include "logstream.h"

using namespace Grammar;

const Pronoun::Persons Pronoun::emptyPersons = { {""} };

Pronoun::~Pronoun()
{
}

PersonalPronoun::PersonalPronoun() : persons(Pronoun::emptyPersons)
{
}

PersonalPronoun::~PersonalPronoun()
{
}

const DLString& PersonalPronoun::decline(const Noun &who, const Person &p, const Case &c) const 
{
    return persons [Person(who.getMultiGender(), p)] [c];
}

const PosessivePronoun::PosessionGenders PosessivePronoun::emptyPosessions = { { { "" } } };

PosessivePronoun::PosessivePronoun() : posessions(PosessivePronoun::emptyPosessions)
{
}

PosessivePronoun::~PosessivePronoun()
{
}

const DLString& PosessivePronoun::decline(const Noun &item, const Noun &owner, const Person &p, const Case &c) const 
{
    return posessions
            [item.getMultiGender()]
                [Person(owner.getMultiGender(), p)]
                    [c];
}

IndefinitePronoun::IndefinitePronoun(const AnimacyCases &acases)
                    : acases(acases)
{
}

IndefinitePronoun::~IndefinitePronoun()
{
}
    
const DLString& IndefinitePronoun::decline(const Case &c, const Animacy &a) const
{
    return acases[a][c];
}

IndefiniteNoun::IndefiniteNoun(const IndefinitePronoun::AnimacyCases &acases, const Animacy &a)
                   : ipron(acases), a(a)
{
}

Gender IndefiniteNoun::getGender() const
{
    return a == Animacy::PERSON ? Gender::MASCULINE : Gender::NEUTER;
}

Number IndefiniteNoun::getNumber() const
{
    return Number::SINGULAR;
}

const DLString& IndefiniteNoun::decline(const Case &c) const
{
    return ipron.decline(c, a);
}

const DLString &IndefiniteNoun::getFullForm() const
{
    return DLString::emptyString;
}


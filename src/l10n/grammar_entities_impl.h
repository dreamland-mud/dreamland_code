/* $Id: grammar_entities_impl.h,v 1.1.2.6 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_GRAMMAR_ENTITIES_IMPL_H
#define L10N_GRAMMAR_ENTITIES_IMPL_H

#include "grammar_entities.h"

using namespace Grammar;

inline Entity::operator const int & () const 
{
    return value;
}

inline Case::Case(int v) 
{
    value = (v >= NONE && v <= MAX) ? v : NONE;
}

inline Case::Case(char c) 
{
    int v = c - '1';
    value = (v >= NONE && v <= MAX) ? v : NONE;
}

inline Animacy::Animacy(int v) 
{
    value = (v >= NONE && v < MAX) ? v : NONE;
}

inline Person::Person(int v) 
{
    value = (v >= FIRST && v < MAX) ? v : FIRST;
}

inline Person::Person(const MultiGender &mg, const Person &p) 
{
    value = resolve(mg.toGender(), mg.toNumber(), p);
}

inline Person::Person(const Gender &g, const Number &n, const Person &p) 
{
    value = resolve(g, n, p);
}

inline int Person::resolve(const Gender &g, const Number &n, const Person &p) 
{
    switch (n) {
    case Number::SINGULAR:
        switch (p) {
        case THIRD:
            switch (g) {
            case Gender::NEUTER: 
                return THIRD_NEUTER;
            case Gender::FEMININE:
                return THIRD_FEMININE;
            default:
                return p;
            }
        default:
            return p;
        }
    default:
        return p;
    }
}

inline Gender::Gender(int v) 
{
    value = (v >= NONE && v < MAX) ? v : MASCULINE;
}

inline Number::Number(int v) 
{
    value = (v >= NONE && v < MAX) ? v : SINGULAR;
}

inline Number::Number(const char * t) 
{
    value = NONE;
    fromString(t);
}

inline const char * const Number::toString() const
{
    switch (value) {
    default:            
    case SINGULAR:  return "s";
    case PLURAL:    return "p";
    };
}

inline void Number::fromString(const char *t)
{
    if (t) 
        switch (t[0]) {
        case 's':   value = SINGULAR; break;
        case 'p':   value = PLURAL; break;
        }
}

inline MultiGender::MultiGender(int v) 
{
    value = (v >= NONE && v < MAX) ? v : NONE;
}

inline MultiGender::MultiGender(const Gender &g, const Number &n) 
{
    value = resolve(g, n);
}

inline MultiGender::MultiGender(const char * t) 
{
    value = NONE;
    fromString(t);
}

inline Gender MultiGender::toGender() const 
{
    switch (value) {
    case NEUTER:    return Gender::NEUTER;
    case FEMININE:  return Gender::FEMININE;
    default:            return Gender::MASCULINE;
    }
}

inline Number MultiGender::toNumber() const 
{
    switch (value) {
    case PLURAL:    return Number::PLURAL;
    default:            return Number::SINGULAR;
    }
}

inline int MultiGender::resolve(const Gender &g, const Number &n) 
{
    switch (n) {
    case Number::SINGULAR:
        switch (g) {
        case Gender::NEUTER:        return NEUTER;
        case Gender::FEMININE:        return FEMININE;
        default:                return MASCULINE;
        }
    case Number::PLURAL: return PLURAL;
    default:             return MASCULINE;
    }
}

inline const char * const MultiGender::toString() const
{
    switch (value) {
    default:            
    case MASCULINE: return "m";
    case FEMININE:  return "f";
    case NEUTER:    return "n";
    case PLURAL:    return "p";
    };
}

inline void MultiGender::fromString(const char *t)
{
    if (t) 
        switch (t[0]) {
        case 'n':   value = NEUTER; break;
        case 'f':   value = FEMININE; break;
        case 'm':   value = MASCULINE; break;
        case 'p':   value = PLURAL; break;
        }
}

#endif

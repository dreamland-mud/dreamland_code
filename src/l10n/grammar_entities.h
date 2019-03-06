/* $Id: grammar_entities.h,v 1.1.2.5 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_GRAMMAR_ENTITIES_H
#define L10N_GRAMMAR_ENTITIES_H

namespace Grammar {

struct Entity {
    inline operator const int & () const;

    int value;
};

struct Case : public Entity {
    enum {
        NONE,
        NOMINATIVE = NONE,
        GENITIVE,
        DATIVE,
        ACCUSATIVE,
        INSTRUMENTAL,
        PREPOSITIONAL,
        MAX,
    };

    inline Case(int v = NONE);
    inline Case(char c);
};

struct Animacy : public Entity {
    enum {
        NONE,
        PERSON = NONE,
        ITEM,
        MAX
    };

    inline Animacy(int v = NONE);
};

struct Gender : public Entity {
    enum {
        NONE,
        NEUTER = NONE,
        MASCULINE,
        FEMININE,
        MAX,
    };
    
    inline Gender(int v = MASCULINE);
};

struct Number : public Entity {
    enum {
        NONE,
        SINGULAR = NONE,
        PLURAL,
        MAX,
    };
    
    inline Number(int v = NONE);
    inline Number(const char *t);
    inline const char * const toString() const;
    inline void fromString(const char *t);
};

struct MultiGender : public Entity {
    enum {
        NONE,
        NEUTER = NONE,
        MASCULINE,
        FEMININE,
        PLURAL,
        MAX
    };
     
    inline MultiGender(int v = MASCULINE);
    inline MultiGender(const char *t);
    inline MultiGender(const Gender &g, const Number &n);
    inline Gender toGender() const;
    inline Number toNumber() const;
    static inline int resolve(const Gender &g, const Number &n);

    inline const char * const toString() const;
    inline void fromString(const char *t);
};

struct Person : public Entity {
    enum {
        FIRST,
        FIRST_SINGULAR = FIRST,
        SECOND,
        SECOND_SINGULAR = SECOND,
        THIRD,
        THIRD_SINGULAR = THIRD,
        THIRD_MASCULINE = THIRD,
        THIRD_FEMININE,
        THIRD_NEUTER,
        FIRST_PLURAL,
        SECOND_PLURAL,
        THIRD_PLURAL,
        MAX,
    };
    
    inline Person(int v);
    inline Person(const Gender &g, const Number &n, const Person &p);
    inline Person(const MultiGender &mg, const Person &p);
    static inline int resolve(const Gender &g, const Number &n, const Person &p);
};

}

#endif

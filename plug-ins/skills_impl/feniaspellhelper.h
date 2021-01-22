#ifndef __FENIASPELLHELPER_H__
#define __FENIASPELLHELPER_H__

class Spell;

class FeniaSpellHelper {
public:
    static void linkWrappers();
    static void extractWrappers();
    static void linkWrapper(Spell *);
    static void extractWrapper(Spell *);
};

#endif 
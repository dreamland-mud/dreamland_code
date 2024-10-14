#ifndef MONEY_UTILS_H
#define MONEY_UTILS_H

class Object;
class Character;

namespace Money {
    void dematerialize( Object *list, int &gold, int &silver );

    Object *create( int gold, int silver );

    DLString describe( int gold, int silver, const Grammar::Case &gcase );

    bool parse( Character *ch, const char *arg, int amount, int &gold, int &silver );

};

#endif

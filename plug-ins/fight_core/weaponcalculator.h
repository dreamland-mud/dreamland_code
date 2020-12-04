#ifndef WEAPON_CALCULATOR_H
#define WEAPON_CALCULATOR_H

#include "bitstring.h"

/** Weapon parameter calculator: figure out v1, v2 and damroll for given tier, level and weapon class. */
struct WeaponCalculator {
    WeaponCalculator(int tier, int level, bitnumber_t wclass, int index_bonus = 0);

    int getValue1() const { return value1; }
    int getValue2() const { return value2; }
    int getDamroll() const { return damroll; }
    int getAve() const { return ave; }
    int getRealAve() const { return real_ave; }

private:
    void calcValue2Range();
    void calcAve();
    void calcValues();
    void calcDamroll();
    int getTierIndex() const;

    int tier;
    int level;
    bitnumber_t wclass;
    int v2_min;
    int v2_max;
    int value1;
    int value2;
    int ave;
    int real_ave;
    int damroll;
    int index_bonus;
};


#endif
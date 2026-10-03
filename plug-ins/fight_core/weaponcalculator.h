#ifndef WEAPON_CALCULATOR_H
#define WEAPON_CALCULATOR_H

#include "bitstring.h"

/** Weapon parameter calculator: figure out v1, v2 and damroll for given tier, level and weapon class. */
struct WeaponCalculator {
    /** ave_mult scales the table ave before the dice are fitted: a two-hander's share. */
    WeaponCalculator(int tier, int level, bitnumber_t wclass, float index_bonus = 0, float ave_mult = 1);

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
    float index_bonus;
    float ave_mult;
};


#endif
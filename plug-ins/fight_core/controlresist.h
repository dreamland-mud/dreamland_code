#ifndef CONTROLRESIST_H
#define CONTROLRESIST_H

class Character;

/*
 * Wait or daze to set on ch when something raises it from `current` to
 * `pulses`. Legends, bosses and champions shrug off lag other characters put
 * on them (keep 1/4, 1/3, 1/2 of the raise, the same scale as .tmp.mob.tierDuration);
 * the lag of their own commands stays whole. Never lowers `current`.
 */
int control_resist( Character *ch, int current, int pulses );

#endif

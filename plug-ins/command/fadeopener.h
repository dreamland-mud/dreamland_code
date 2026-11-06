#ifndef FADEOPENER_H
#define FADEOPENER_H

class Character;

/**
 * The character whose outermost command was dispatched while faded, and the
 * interpret_serial() that command runs under. Fenia reads the pair as
 * ch.fadeBroken (fade+ sudden death reroll): by the time a skill rolls, fade
 * is either stripped already (Command::visualize) or about to be (set_fighting).
 */
extern Character *fade_broken_by;
extern long fade_broken_serial;

#endif

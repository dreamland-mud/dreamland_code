#ifndef FADEOPENER_H
#define FADEOPENER_H

class Character;

/**
 * The character whose command is being dispatched right now, if it came out of
 * fade to run it (Command::visualize strips fade first). murder reads it to
 * grant a fade+ opener, since fade is already gone by the time it runs.
 */
extern Character *fade_broken_by;

#endif

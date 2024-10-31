#ifndef RESETS_H
#define RESETS_H

#include "flags.h"
#include "enumeration.h"

struct reset_data;

typedef struct reset_data RESET_DATA;

typedef vector<reset_data *> ResetList;

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Room reset definition.
 */
struct reset_data
{
    reset_data();

    char command;
    int arg1;
    int arg2;
    int arg3;
    int arg4;

    Flags flags;
    Enumeration rand;
    int bestTier;
    vector<int> vnums;
};


#endif
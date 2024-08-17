/*
 * (c) https://github.com/git/git/blob/master/levenshtein.h
 */
#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

class DLString;

int levenshtein(const DLString &string1, const DLString &string2,
    int swap_penalty, int substitution_penalty,
    int insertion_penalty, int deletion_penalty);

#endif

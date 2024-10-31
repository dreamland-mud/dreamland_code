#ifndef AUCTION_H
#define AUCTION_H

class Character;
class Object;

typedef struct auction_data AUCTION_DATA; 

/*
 * auction data
 */
struct  auction_data
{
    auction_data( );

    Object * item;   /* a pointer to the item */
    Character * seller; /* a pointer to the seller - which may NOT quit */
    Character * buyer;  /* a pointer to the buyer - which may NOT quit */
    int bet;    /* last bet - or 0 if noone has bet anything */
    int startbet;
    int going;  /* 1,2, sold */
    int pulse;  /* how many pulses (.25 sec) until another call-out ? */
};

extern AUCTION_DATA * auction;

#endif

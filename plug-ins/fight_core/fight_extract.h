#ifndef FIGHT_EXTRACT_H
#define FIGHT_EXTRACT_H

class Character;
class PCharacter;

#define    FEXTRACT_TOTAL  (A)
#define    FEXTRACT_COUNT  (B)
#define    FEXTRACT_LASTFOUGHT  (C)

void  extract_char( Character *, bool fCount = true );

void extract_dead_player( PCharacter *ch, int flags );

void nuke_pets( PCharacter *ch, int flags );

void notify_referers( Character *ch, int flags );


#endif
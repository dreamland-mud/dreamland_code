#ifndef DOORS_H
#define DOORS_H

class Character;
class Object;

void open_door_extra ( Character *ch, int door, void *pexit );
void open_door( Character *, int );
bool open_portal( Character *, Object * );

#endif

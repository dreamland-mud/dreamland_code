#ifndef SCREENREADER_H
#define SCREENREADER_H

class Descriptor;
class Character;

bool uses_screenreader(Descriptor *d);
bool uses_screenreader(Character *ch);

#endif

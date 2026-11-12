#ifndef SCREENREADER_H
#define SCREENREADER_H

class Descriptor;
class Character;
class DLString;

bool uses_screenreader(Descriptor *d);
bool uses_screenreader(Character *ch);

// Drop decorative symbols from an object name for screen readers:
// "-=Blade of Shadow=-" reads as "Blade of Shadow". Colour codes are kept.
DLString screenreader_clean_name(const DLString &name);

#endif

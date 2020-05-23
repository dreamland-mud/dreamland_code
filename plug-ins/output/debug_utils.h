#ifndef DEBUG_UTILS_H
#define DEBUG_UTILS_H

#include <sstream>

using namespace std;

class Character;

struct Debug {
    Debug(Character *_ch, const char *_attr, const char *_label);

    ~Debug();

    Debug & log(float chance, const char *msg);

    ostringstream buf;
    Character *ch;
    const char *attr;
    const char *label;
};

#endif

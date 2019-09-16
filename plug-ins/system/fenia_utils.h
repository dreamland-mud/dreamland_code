#ifndef FENIA_UTILS
#define FENIA_UTILS

class DLString;

/** Call a global Fenia trigger with given name, argument format and arguments. */
bool gprog(const DLString &trigName, const char *fmt, ...);

#endif


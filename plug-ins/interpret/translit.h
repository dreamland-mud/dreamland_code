#ifndef TRANSLIT_H
#define TRANSLIT_H

class DLString;

char translit(char c0);
DLString translit(lang_t lang, const DLString &s);

#endif

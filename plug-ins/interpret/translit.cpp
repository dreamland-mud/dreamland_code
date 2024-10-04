#include <sstream>
#include "dl_ctype.h"
#include "dlstring.h"

char translit(char c0)
{
    char c = dl_tolower(c0);

    switch(c) {
    case 'a':   return 'ф';
    case 'b':   return 'и';   
    case 'c':   return 'с';
    case 'd':   return 'в';
    case 'e':   return 'у';
    case 'f':   return 'а';
    case 'g':   return 'п';
    case 'h':   return 'р';
    case 'i':   return 'ш';
    case 'j':   return 'о';
    case 'k':   return 'л';
    case 'l':   return 'д';
    case 'm':   return 'ь';
    case 'n':   return 'т';
    case 'o':   return 'щ';
    case 'p':   return 'з';
    case 'q':   return 'й';
    case 'r':   return 'к';
    case 's':   return 'ы';
    case 't':   return 'е';
    case 'u':   return 'г';
    case 'v':   return 'м';
    case 'w':   return 'ц';
    case 'x':   return 'ч';
    case 'y':   return 'н';
    case 'z':   return 'я';
    case '[':   return 'х';
    case '{':   return 'х';
    case ']':   return 'ъ';
    case '}':   return 'ъ';
    case ';':   return 'ж';
    case ':':   return 'ж';
    case '\'':   return 'э';
    case '"':   return 'э';
    case ',':   return 'б';
    case '<':   return 'б';
    case '.':   return 'ю';
    case '>':   return 'ю';
    case '/':   return '.';
    case '?':   return '.';
    case '`':   return 'ё';
    case '~':   return 'ё';

    case 'й':    return 'q';
    case 'ц':    return 'w';
    case 'у':    return 'e';
    case 'к':    return 'r';
    case 'е':    return 't';
    case 'н':    return 'y';
    case 'г':    return 'u';
    case 'ш':    return 'i';
    case 'щ':    return 'o';
    case 'з':    return 'p';
    case 'х':    return '[';
    case 'ъ':    return ']';
    case 'ф':    return 'a';
    case 'ы':    return 's';
    case 'в':    return 'd';
    case 'а':    return 'f';
    case 'п':    return 'g';
    case 'р':    return 'h';
    case 'о':    return 'j';
    case 'л':    return 'k';
    case 'д':    return 'l';
    case 'ж':    return ';';
    case 'э':    return '\'';
    case 'ё':    return '`';
    case 'я':    return 'я';
    case 'ч':    return 'x';
    case 'с':    return 'c';
    case 'м':    return 'v';
    case 'и':    return 'b';
    case 'т':    return 'n';
    case 'ь':    return 'm';
    case 'б':    return ',';
    case 'ю':    return '.';
    }
    return c;
}

// TODO add UA characters
DLString translit(const DLString &s)
{
    ostringstream buf;
    for (size_t i = 0; i < s.size(); i++)
        buf << translit(s.at(i));
    return buf.str();
}



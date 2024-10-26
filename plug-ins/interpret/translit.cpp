#include <sstream>
#include "dl_ctype.h"
#include "dlstring.h"
#include "lang.h"

static char en2ua(char c0)
{
    switch(c0) {
    case 's':   return 'і';
    case ']':   return 'ї';
    case '}':   return 'ї';
    case '\'':   return 'є';
    case '"':   return 'є';
    case '`':   return 'ґ';
    case '~':   return 'ґ';
    default: return c0;
    }
}

static char en2ru(char c0)
{
    switch(c0) {
    case 's':   return 'ы';
    case ']':   return 'ъ';
    case '}':   return 'ъ';
    case '\'':   return 'э';
    case '"':   return 'э';
    case '`':   return 'ё';
    case '~':   return 'ё';
    default: return c0;
    }
}

char translit(lang_t lang, char c)
{
    char c0 = dl_tolower(c);
    char c1 = lang == UA ? en2ua(c0) : en2ru(c0);
    if (c0 != c1)
        return c1;

    switch(c0) {
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
    case 't':   return 'е';
    case 'u':   return 'г';
    case 'v':   return 'м';
    case 'w':   return 'ц';
    case 'x':   return 'ч';
    case 'y':   return 'н';
    case 'z':   return 'я';
    case '[':   return 'х';
    case '{':   return 'х';
    case ';':   return 'ж';
    case ':':   return 'ж';
    case ',':   return 'б';
    case '<':   return 'б';
    case '.':   return 'ю';
    case '>':   return 'ю';
    case '/':   return '.';
    case '?':   return '.';

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
    case 'є':    return '\'';
    case 'ґ':    return '`';
    case 'ї':    return ']';
    case 'і':    return 's';
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

DLString translit(lang_t lang, const DLString &s)
{
    ostringstream buf;
    for (size_t i = 0; i < s.size(); i++)
        buf << translit(lang, s.at(i));
    return buf.str();
}



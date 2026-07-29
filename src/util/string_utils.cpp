#include <sstream>
#include "string_utils.h"
#include "dl_ctype.h"
#include "xmlmultistring.h"
#include "logstream.h"
#include "grammar_entities_impl.h"

using namespace std;

bool String::equalLess(const DLString &a, const DLString &b)
{
    if (a.length( ) != b.length( ))
        return false;

    for (DLString::size_type pos = 0; pos < b.length(); pos++) 
        if( dl_tolower(a.at( pos ) ) != dl_tolower( b.at( pos ) ) )
                return false;

    return true;
}

DLString String::truncate(const DLString &constString, size_t size)
{
    DLString str = constString;

    if (str.length() > size)
        str.erase(size);

    return str;
}

DLString String::ellipsis(const DLString& constString, size_t size)
{
    DLString str = truncate(constString, size);
    bool addEllipsis = constString.size() != str.size();

    str = stripEOL(str);

    if (addEllipsis)
        str << "...";

    return str;
}

DLString String::stripEOL(const DLString& constString)
{
    DLString str = constString;

    auto lf_pos = str.find_last_not_of('\n');
    if (lf_pos != DLString::npos)
        str.erase(lf_pos + 1);

    return str;
}

bool String::hasUaSymbol(const DLString &str)
{
    for (DLString::size_type pos = 0; pos < str.length(); pos++) 
        if (dl_is_ukr_specific(str.at(pos)))
            return true;

    return false;
}

bool String::hasRuSymbol(const DLString &str)
{
    for (DLString::size_type pos = 0; pos < str.length(); pos++) 
        if (dl_is_rus_specific(str.at(pos)))
            return true;

    return false;
}

bool String::hasCyrillic(const DLString &str)
{
    if (str.empty( ))
        return false;

    for (DLString::size_type i = 0; i < str.length( ); i++) 
        if (dl_is_cyrillic(str.at(i)))
            return true;

    return false;
}

// One lower-case Cyrillic letter -> its Latin spelling. Sources are UTF-8 but
// the build's -fexec-charset=KOI8-U folds each literal to a single runtime byte,
// so a plain char switch matches the KOI8 characters a name is made of. Digraphs
// for hushers and iotated vowels; hard/soft signs drop; Russian and Ukrainian
// share the table (г->g, і/и->i) since a bare name gives no language to
// disambiguate on. Returns 0 for a non-Cyrillic byte (pass it through), "" for a
// sign (drop it). Uppercase 'Ъ' is listed explicitly: it sits at the end of the
// KOI8 upper range that dl_tolower folds and would otherwise slip through.
static const char * cyr_letter_to_latin(char c)
{
    switch (c) {
    case 'а': return "a";  case 'б': return "b";  case 'в': return "v";
    case 'г': return "g";  case 'ґ': return "g";  case 'д': return "d";
    case 'е': return "e";  case 'є': return "ye"; case 'ё': return "yo";
    case 'ж': return "zh"; case 'з': return "z";  case 'и': return "i";
    case 'і': return "i";  case 'ї': return "yi"; case 'й': return "y";
    case 'к': return "k";  case 'л': return "l";  case 'м': return "m";
    case 'н': return "n";  case 'о': return "o";  case 'п': return "p";
    case 'р': return "r";  case 'с': return "s";  case 'т': return "t";
    case 'у': return "u";  case 'ф': return "f";  case 'х': return "kh";
    case 'ц': return "ts"; case 'ч': return "ch"; case 'ш': return "sh";
    case 'щ': return "shch"; case 'ъ': return "";  case 'Ъ': return "";
    case 'ы': return "y";  case 'ь': return "";   case 'э': return "e";
    case 'ю': return "yu"; case 'я': return "ya";
    }
    return 0;
}

DLString String::translitToLatin(const DLString &str)
{
    ostringstream buf;
    bool converted = false;

    for (DLString::size_type i = 0; i < str.length(); i++) {
        const char *lat = cyr_letter_to_latin( dl_tolower( str.at(i) ) );
        if (lat != 0) {
            buf << lat;             // Cyrillic letter -> Latin (digraph, or "" for signs)
            converted = true;
        } else {
            buf << str.at(i);       // ASCII / punctuation / non-Cyrillic byte, verbatim
        }
    }

    DLString out = buf.str();
    // Capitalize the single-token name -- but only when we actually romanized,
    // so an all-Latin login is returned byte-identical.
    if (converted && !out.empty() && out.at(0) >= 'a' && out.at(0) <= 'z')
        out.at(0) = out.at(0) - ('a' - 'A');
    return out;
}

// Some personal names do not decline at all, and a pad repeating the nominative
// is the CORRECT answer for them, not a failure to inflect: foreign names ending
// in a vowel (Кворо, Самуро, Теруто) and feminine names ending in a consonant
// (Тайфоэн, Кармен) keep one form in every case. These are exactly the classes
// the rule-based Russian decliner in fenia utils/inflect already refuses.
//
// -а and -я are NOT here: those decline in both languages (Олена -> Олени).
// KOI8 char switch, as everywhere else in this file.
bool String::nameIsIndeclinable(const DLString &name, bool female)
{
    if (name.size( ) <= 2)
        return true;

    char last = dl_tolower( name.at( name.size( ) - 1 ) );

    switch (last) {
    case 'о': case 'у': case 'ю': case 'е': case 'є':
    case 'и': case 'і': case 'ї': case 'э': case 'ы': case 'ё':
        return true;
    }

    // A feminine name ending in a consonant. 'ь' is not a consonant for this
    // purpose -- Любовь and friends still decline.
    if (female && dl_is_cyrillic( last )) {
        switch (last) {
        case 'а': case 'я': case 'ь':
            return false;
        }
        return true;
    }

    return false;
}

// The four Russian letters Ukrainian does not have. Same KOI8 note as
// cyr_letter_to_latin above: -fexec-charset=KOI8-U folds each literal to one
// runtime byte, so this is a char switch and not UTF-8. 'ъ' drops, 'ё' expands
// to a digraph. Returns 0 for anything else, meaning "pass the byte through".
static const char * ru_letter_to_ua(char c)
{
    switch (c) {
    case 'э': return "е";   case 'Э': return "Е";
    case 'ы': return "и";   case 'Ы': return "И";
    case 'ъ': return "";    case 'Ъ': return "";
    case 'ё': return "йо";  case 'Ё': return "Йо";
    }
    return 0;
}

DLString String::ruLettersToUa(const DLString &str)
{
    ostringstream buf;

    for (DLString::size_type i = 0; i < str.length(); i++) {
        const char *ua = ru_letter_to_ua( str.at(i) );
        if (ua != 0)
            buf << ua;
        else
            buf << str.at(i);
    }

    return buf.str();
}

bool String::lessCase( const DLString &a, const DLString& b )
{
        DLString::size_type len = a.length( ) < b.length( ) ? a.length( ) : b.length( );
        for( DLString::size_type i = 0; i < len; i++ )
        {
                char ch1 = dl_toupper( a.at( i ) );
                char ch2 = dl_toupper( b.at(i) );
                if( ch1 < ch2 )
                {
                        return true;
                }
                else if( ch1 > ch2 )
                {
                        return false;
                }
        }
        if( a.length( ) < b.length( ) )
        {
                return true;
        }
        return false;
}

bool String::isEmpty( const char *arg )
{
    DLString descr;

    if (arg == 0 || arg[0] == 0)
        return true;

    descr = arg;
    descr.colourstrip( );
    
    if (descr.empty( ))
        return true;

    return false;
}

const DLString& String::firstNonEmpty(const XMLMultiString& a, const XMLMultiString& b, lang_t lang)
{
    const DLString &a_value = a.get(lang);
    if (!a_value.empty())
        return a_value;

    const DLString &b_value = b.get(lang);
    if (!b_value.empty())
        return b_value;

    return DLString::emptyString;
}

DLString String::addLine(const DLString& text, const DLString& line)
{
    ostringstream buf;

    buf << text << line << endl;

    return buf.str();
}

DLString String::delLine(const DLString& text)
{
    DLString buf = text;
    DLString::size_type i1, i2;

    if (buf.empty())
        return buf;
    
    i1 = buf.find_last_of( "\n" );

    if (i1 == DLString::npos || i1 == 0) {
        buf.erase( );
    }
    else {
        i2 = buf.find_last_of( "\n", i1 - 1 );
        
        if (i2 == DLString::npos) 
            buf.erase( );
        else
            buf.erase( i2 + 1 );
    }

    return buf;
}

bool String::contains(const DLString& bigString, const DLString& smallString)
{
    return bigString.find(smallString) != DLString::npos;
}

std::list<DLString> String::toLines(const DLString& text)
{
    char buf[1024];
    list<DLString> result;

    if (text.empty())
        return result;

    istringstream is(text);
    while (is.getline(buf, sizeof(buf)))
        result.push_back(buf);

    return result;
}

DLString String::fromLines(const std::list<DLString>& lines)
{
    ostringstream buf;

    for (auto &l: lines)
        buf << l << endl;

    return buf.str();
}

DLString String::join(const std::list<DLString>& list, const DLString& delim)
{
    ostringstream buf;

    for (auto i = list.begin(); i != list.end(); i++) {
        if (i != list.begin())
            buf << delim;

        buf << *i;
    }

    return buf.str();
}

StringList String::getAllForms(const XMLMultiString& multiString)
{
    StringList forms;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        DLString lname = multiString.get(lang).toLower();

        if (lname.find('|') != DLString::npos) {
            for (int gcase = Case::NOMINATIVE; gcase < Case::MAX; gcase++)
                forms.push_back(lname.ruscase('1' + gcase));
        } else {
            forms.push_back(lname);
        }
    }

    return forms;
}

DLString String::toString(const XMLMultiString& multiString)
{
    return getAllForms(multiString).join(" ");
}

std::list<DLString> String::toNormalizedList(const XMLMultiString& multiString)
{
    std::list<DLString> result;

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        const DLString &value = multiString.get(lang);
        
        if (!value.empty())
            result.push_back(value.ruscase('1').colourStrip());
    }

    return result;
}


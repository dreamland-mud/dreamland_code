#include <jsoncpp/json/json.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include "logstream.h"
#include "morphology.h"
#include "grammar_entities_impl.h"
#include "configurable.h"
#include "stringset.h"
#include "stringlist.h"
#include "dl_ctype.h"
#include "iconvmap.h"
#include "flagmessagestore.h"

Json::Value rules;
CONFIGURABLE_LOADED(grammar, rules)
{
    rules = value;
}

// Externalized per-language flag-table messages (bits.conf Variant 2), defined
// in config/flagmessages.json. Shape:
//   { "<table>": { "<flag>": { "en": "...", "ru": "...", "ua": "..." } } }
// Every entry is optional; a missing (table, flag, lang) falls back inside
// FlagTable to the RU entry, then to the in-binary message, so a partial file
// is always safe. See flagmessagestore.h.
CONFIGURABLE_LOADED(config, flagmessages)
{
    static const struct { const char *key; lang_t lang; } LANGS[] = {
        { "en", LANG_EN }, { "ru", LANG_RU }, { "ua", LANG_UA },
    };

    FlagMessageStore &store = FlagMessageStore::shared( );
    store.clear( );

    for (const auto &tableName: value.getMemberNames( )) {
        const Json::Value &flags = value[tableName];
        if (!flags.isObject( ))
            continue;

        for (const auto &flagName: flags.getMemberNames( )) {
            const Json::Value &langs = flags[flagName];
            if (!langs.isObject( ))
                continue;

            for (const auto &L: LANGS) {
                const Json::Value &pad = langs[L.key];
                if (pad.isString( ))
                    store.set( tableName, flagName, L.lang, pad.asString( ) );
            }
        }
    }

    LogStream::sendNotice( ) << "flagmessages: loaded " << value.getMemberNames( ).size( )
                             << " flag tables into the message store." << endl;
}

static vector<DLString> split(const DLString &s, char delim)
{
    vector<DLString> result;
    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos)
    {
        result.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(delim, start);
    }

    result.push_back(s.substr(start, end));
    return result;
}

DLString Morphology::adjective(const DLString &form, const MultiGender &gender)
{
    DLString::size_type open_bracket = form.find_first_of('(');
    DLString::size_type close_bracket = form.find_last_of(')');

    if (open_bracket == DLString::npos
        || close_bracket == DLString::npos
        || close_bracket != form.length() - 1
        || close_bracket <= open_bracket)
    {
        warn("morphology: form %s doesn't have endings", form.c_str());
        return form;
    }

    DLString stem = form.substr(0, open_bracket);
    DLString endingsWithComma = form.substr(open_bracket + 1, close_bracket - open_bracket - 1);
    vector<DLString> endings = split(endingsWithComma, ',');

    if ((int)endings.size() <= (int)gender) {
        warn("morphology: %d endings in form %s do not match gender %d", (int)endings.size(), form.c_str(), (int)gender);
        return form;
    }

    DLString rule = endings[gender];
    if (rules[rule].empty()) {
        warn("morphology: rule %s not found", rule.c_str());
        return form;
    }

    DLString cases = rules[rule].asString();
    return stem + cases;
}

static DLString decline_sidecar( const DLString &word, const DLString &pos, const DLString &gender, const DLString &lang )
{
    DLString fallback = word + "|||||"; // nominative in every case, no delta

    if (word.empty( ))
        return fallback;

    static std::map<DLString, DLString> cache;
    DLString key = word + "\t" + pos + "\t" + gender + "\t" + lang;
    std::map<DLString, DLString>::iterator ci = cache.find( key );
    if (ci != cache.end( ))
        return ci->second;

    static IconvMap koi2utf( "koi8-u", "utf-8" );
    static IconvMap utf2koi( "utf-8", "koi8-u" );

    int fd = ::socket( AF_INET, SOCK_STREAM, 0 );
    if (fd < 0)
        return fallback;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; // 500ms cap -- authoring-time, never a hot path
    ::setsockopt( fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv) );
    ::setsockopt( fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv) );

    struct sockaddr_in addr;
    memset( &addr, 0, sizeof(addr) );
    addr.sin_family = AF_INET;
    addr.sin_port = htons( 5299 );
    inet_pton( AF_INET, "127.0.0.1", &addr.sin_addr );

    if (::connect( fd, (struct sockaddr *)&addr, sizeof(addr) ) < 0) {
        ::close( fd );
        return fallback;
    }

    DLString req = koi2utf( word ) + "\t" + pos + "\t" + gender + "\t" + lang + "\n";
    if (::write( fd, req.c_str( ), req.size( ) ) < 0) {
        ::close( fd );
        return fallback;
    }

    char buf[4096];
    int n = ::read( fd, buf, sizeof(buf) - 1 );
    ::close( fd );
    if (n <= 0)
        return fallback;
    buf[n] = '\0';

    DLString resp = utf2koi( std::string( buf ) ); // "pad\tscore\n"
    DLString::size_type tab = resp.find( '\t' );
    if (tab == DLString::npos)
        return fallback;

    DLString pad = resp.substr( 0, tab );
    double score = atof( resp.substr( tab + 1 ).c_str( ) );
    if (score < 0.4)
        LogStream::sendWarning( ) << "ua-morph: low confidence " << score
                                  << " declining '" << word << "' -> " << pad << endl;

    cache[key] = pad;
    return pad;
}

DLString Morphology::declineUa( const DLString &word, const DLString &pos, const DLString &gender )
{
    return decline_sidecar( word, pos, gender, "uk" );
}

DLString Morphology::declineRu( const DLString &word, const DLString &pos, const DLString &gender )
{
    return decline_sidecar( word, pos, gender, "ru" );
}

static bool is_consonant(char c)
{
    return c == 'б' || c == 'в' || c == 'г' || c == 'д' || c == 'ж' || c == 'з' || c == 'й' || 
    c == 'к' || c == 'л' || c == 'м' || c == 'н' || c == 'п' || c == 'р' || c == 'с' || 
    c == 'т' || c == 'ф' || c == 'х' || c == 'ц' || c == 'ч' || c == 'ш' || c == 'щ' ||
    c == 'ь' || c == 'ъ';
}

DLString Morphology::preposition_with(const DLString &noun)
{
    DLString s = "с", so = "со";
    DLString n = noun.toLower();

    if (n.empty())
        return s;
    
    char firstLetter = n.at(0);
    if (firstLetter == 'щ')
        return so;

    if (n.size() <= 1)
        return s;

    char nextLetter = n.at(1);
    if (firstLetter == 'с' || firstLetter == 'ж' || firstLetter == 'ш' || firstLetter == 'з')
        if (is_consonant(nextLetter))
            return so;

    return s;
}

// Ukrainian euphony works the same way as the Russian rule above -- the
// preposition grows a vowel in front of a consonant cluster -- so the shape is
// deliberately identical, only the words differ ("з"/"зі" instead of "с"/"со").
// is_consonant() lists the Cyrillic consonants both alphabets share; the letters
// unique to Ukrainian (і, ї, є) are vowels and correctly fall through.
static DLString preposition_with_ua(const DLString &noun)
{
    DLString s = "з", so = "зі";
    DLString n = noun.toLower();

    if (n.empty())
        return s;

    char firstLetter = n.at(0);
    if (firstLetter == 'щ')
        return so;

    if (n.size() <= 1)
        return s;

    char nextLetter = n.at(1);
    if (firstLetter == 'с' || firstLetter == 'ж' || firstLetter == 'ш' || firstLetter == 'з')
        if (is_consonant(nextLetter))
            return so;

    return s;
}

DLString Morphology::preposition_with(lang_t lang, const DLString &noun)
{
    switch (lang) {
    case LANG_EN:
        // English has no euphonic variant to choose; the noun is irrelevant.
        return "from";
    case LANG_UA:
        return preposition_with_ua(noun);
    default:
        return preposition_with(noun);
    }
}

DLString Syntax::noun(const DLString &phrase)
{
    StringList words(phrase.colourStrip().toLower());
    if (words.empty())
        return phrase;

    // In case of several words, look for the last one with gram case endings.
    for (auto w = words.rbegin(); w != words.rend(); w++)
        if (w->find('|') != DLString::npos)
            return *w;

    // For everything else just return the last word.
    return words.back();
}

DLString Syntax::label(const DLString &names)
{
    DLString rus = label_ru(names);
    DLString eng = label_en(names);

    if (rus.empty()) rus = eng;
    if (eng.empty()) eng = rus;
    
    if (eng.empty() && rus.empty()) 
        return DLString::emptyString;

    return "" + rus + "";
}

/** Return true if string doesn't contain any RU characters. */
static bool name_is_en(const DLString &name)
{
    for (unsigned int i = 0; i < name.size(); i++)
        if (dl_is_cyrillic(name.at(i)))
            return false;
    return true;
}

DLString Syntax::label_en(const DLString &names)
{
    StringList labels(names.colourStrip());

    for (auto &l: labels)
        if (name_is_en(l))
            return l;

    return DLString::emptyString;
}

DLString Syntax::label_ru(const DLString &names)
{
    StringList labels(names.colourStrip());

    for (auto &l: labels)
        if (l.isCyrillic())
            return l;

    return DLString::emptyString;
}


DLString Syntax::label_en(const XMLMultiString &str)
{
    DLString label = str.get(LANG_EN);
    if (label.empty())
        label = str.get(LANG_RU);
        
    return label.getOneArgument();
}

DLString Syntax::label_ru(const XMLMultiString &str)
{
    DLString label = str.get(LANG_RU);
    if (label.empty())
        label = str.get(LANG_EN);
    
    return label.getOneArgument();
}

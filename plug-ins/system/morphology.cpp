#include <jsoncpp/json/json.h>
#include "logstream.h"
#include "morphology.h"
#include "grammar_entities_impl.h"
#include "configurable.h"
#include "stringset.h"
#include "stringlist.h"
#include "dl_ctype.h"

Json::Value rules;
CONFIGURABLE_LOADED(grammar, rules)
{
    rules = value;
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

    return "{lE" + eng + "{lR" + rus + "{lx";
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

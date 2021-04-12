#include <jsoncpp/json/json.h>
#include "logstream.h"
#include "morphology.h"
#include "grammar_entities_impl.h"
#include "configurable.h"
#include "stringset.h"
#include "stringlist.h"

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

DLString Syntax::noun(const DLString &phrase) 
{
    StringList words(phrase.colourStrip().toLower());
    if (words.empty())
        return phrase;

    // Recognize "наемник из герихельма".
    StringSet preps("из на");
    for (auto &w: words)
        if (preps.count(w) > 0)
            return words.front();

    // For everything else just return the last word.
    return words.back();
}

DLString Syntax::label(const DLString &names)
{
    StringList labels(names);
    DLString rus, eng;

    for (auto &l: labels) {
        if (l.isRussian()) {
            if (rus.empty()) rus = l;
        } else {
            if (eng.empty()) eng = l;
        }
    }

    if (rus.empty()) rus = eng;
    if (eng.empty()) eng = rus;
    
    if (eng.empty() && rus.empty()) 
        return DLString::emptyString;

    return "{lE" + eng + "{lR" + rus + "{lx";
}

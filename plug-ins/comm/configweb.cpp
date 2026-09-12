/* Settings dialog support for the web client: the schema of all settings, their
 * current values, and one way to change them.
 *
 * The dialog draws itself from what the server tells it, so a setting added to
 * the world shows up in it without touching the client. Three frames carry that:
 *
 *   config_schema   asked once, answered with every setting, its type and the
 *                   words to show for it in the player's own language;
 *   prompt.config   the current values, sent whenever one of them changes;
 *   config_set      a change, applied through the same code the 'config'
 *                   command uses, so the player gets the usual reply.
 *
 * An older client asks for none of this and is none the wiser; a newer client
 * asking an older server gets no answer to config_schema and says so.
 */
#include <algorithm>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include <jsoncpp/json/json.h>

#include "configs.h"
#include "configweb.h"

#include "rpccommandmanager.h"
#include "webprompt.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "player_utils.h"
#include "logstream.h"
#include "mudtags.h"
#include "msgformatter.h"
#include "l10n.h"
#include "lang.h"
#include "merc.h"
#include "def.h"

static const DLString CONFIG = "config";

/*-------------------------------------------------------------------------
 * Text, in the language this player reads
 *------------------------------------------------------------------------*/

/** Plain words, the way a terminal without colours would print them. The
 *  world's texts carry the game's own markup -- colour codes and links into the
 *  help -- and the dialog has neither a palette for the one nor a place to go
 *  for the other, so both are resolved away here rather than leaking onto the
 *  screen as '{hh96'. */
static DLString plain_text(const DLString &source, Character *ch)
{
    if (source.empty())
        return source;

    ostringstream buf;
    // NOWEB drops the help links and their labels' markup, NOCOLOR takes the
    // colours out, and RAW keeps the stripping from writing an ANSI 'clear'
    // sequence in their place -- without it the plain text ends in an escape
    // sequence the dialog has no business showing.
    mudtags_convert(source.c_str(), buf,
                    TAGS_CONVERT_VIS|TAGS_ENFORCE_NOWEB|
                    TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR|TAGS_ENFORCE_RAW, ch);
    return buf.str();
}

/** One piece of text, in the language this player reads. The schema is sent in
 *  that language alone: all three at once made it three times the size, and a
 *  frame that big is the one the server truncates on a slow link. The dialog
 *  asks again when the player changes language, which happens far less often
 *  than it opens.
 *
 *  Text the world has nothing for in any language comes back empty, and the
 *  dialog shows nothing rather than a blank line. */
static DLString json_text(const XMLMultiString &text, lang_t lang)
{
    const DLString &value = text.get(lang);
    if (!value.empty())
        return value;

    // Not the general fallback of getForLang(): a sample of English output has
    // no business appearing under a Russian setting. Only a real translation
    // counts, except for the Ukrainian which has always leaned on the Russian.
    if (lang == LANG_UA)
        return text.get(RU);

    return DLString::emptyString;
}

/** Option names live in three separate fields rather than a multi-string, and
 *  a missing Ukrainian name has always fallen back to the Russian one. */
static DLString json_name(const ConfigOption *option, lang_t lang)
{
    if (lang == LANG_EN)
        return option->getName();

    if (lang == LANG_UA && !option->getUaName().empty())
        return option->getUaName();

    return option->getRussianName();
}

/** Every name this thing answers to, in all three languages at once, for the
 *  dialog's search box. The schema itself travels in one language -- that is
 *  what keeps it small -- but a player who knows a setting as 'кратко' should
 *  find it with an English interface, and the other way round. Names are short,
 *  so all three cost a few dozen bytes. */
static DLString search_words(const ConfigOption *option)
{
    DLString words = option->getName();

    if (!option->getRussianName().empty())
        words << " " << option->getRussianName();

    if (!option->getUaName().empty() && option->getUaName() != option->getRussianName())
        words << " " << option->getUaName();

    return words;
}

static DLString search_words(const XMLMultiString &text)
{
    DLString words;

    for (int lang = LANG_MIN; lang < LANG_MAX; lang++) {
        const DLString &value = text.get((lang_t)lang);
        if (value.empty() || words.find(value) != DLString::npos)
            continue;

        if (!words.empty())
            words << " ";

        words << value;
    }

    return words;
}

/** Samples of game output, converted to the markup the web client already
 *  renders everywhere else, so the help shows the line exactly as the terminal
 *  would. Colours are enforced: the sample has to look the same whether or not
 *  this player has colours on.
 *
 *  A sample the world has only in another language is left out: it would teach
 *  this player nothing. */
static Json::Value json_examples(const ConfigOption *option, Character *ch, lang_t lang)
{
    Json::Value result;

    for (auto &example: option->getExamples()) {
        DLString source = json_text(example.getText(), lang);
        if (source.empty())
            continue;

        ostringstream buf;
        mudtags_convert(source.c_str(), buf,
                        TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_WEB, ch);

        Json::Value one;
        one["label"] = json_text(example.getLabel(), lang).c_str();
        one["text"] = buf.str();
        result.append(one);
    }

    return result;
}

/** What every option carries no matter how it is stored. */
static Json::Value json_option(const ConfigOption *option, Character *ch, lang_t lang)
{
    Json::Value result;

    result["key"] = option->getName().c_str();
    result["label"] = json_name(option, lang).c_str();
    result["search"] = search_words(option).c_str();

    DLString hint = plain_text(json_text(option->getHint(), lang), ch);
    if (!hint.empty())
        result["hint"] = hint.c_str();

    DLString help = plain_text(json_text(option->getHelp(), lang), ch);
    if (!help.empty())
        result["help"] = help.c_str();

    Json::Value examples = json_examples(option, ch, lang);
    if (!examples.empty())
        result["examples"] = examples;

    return result;
}

/** A flag option. Under it the dialog shows what the game says about the world
 *  in its current state -- the same sentence the terminal prints when the flag
 *  is flipped, which is why both are sent. */
static Json::Value json_flag_option(const ConfigElement *option, Character *ch, lang_t lang)
{
    Json::Value result = json_option(option, ch, lang);

    result["type"] = "bool";

    DLString on = plain_text(json_text(option->getMsgOn(), lang), ch);
    if (!on.empty())
        result["on"] = on.c_str();

    DLString off = plain_text(json_text(option->getMsgOff(), lang), ch);
    if (!off.empty())
        result["off"] = off.c_str();

    return result;
}

static Json::Value json_enum_values(const ConfigValueElement *option, Character *ch, lang_t lang)
{
    Json::Value result;

    for (auto &value: option->getValues()) {
        Json::Value one;
        one["value"] = value.getValue().c_str();
        one["label"] = json_text(value.getLabel(), lang).c_str();

        DLString desc = plain_text(json_text(value.getDescription(), lang), ch);
        if (!desc.empty())
            one["desc"] = desc.c_str();

        result.append(one);
    }

    return result;
}

/** An option holding a value: what kind it is, what it will accept, and how to
 *  say out loud what it holds. The sentence carries %d or %s where the value
 *  goes, and a second one covers an option holding nothing. */
static Json::Value json_value_option(const ConfigValueElement *option, Character *ch, lang_t lang)
{
    Json::Value result = json_option(option, ch, lang);
    const DLString &type = option->getWebType();

    result["type"] = type.empty() ? "string" : type.c_str();

    DLString desc = plain_text(json_text(option->getDescription(), lang), ch);
    if (!desc.empty())
        result["desc"] = desc.c_str();

    DLString descEmpty = plain_text(json_text(option->getEmptyDescription(), lang), ch);
    if (!descEmpty.empty())
        result["descEmpty"] = descEmpty.c_str();

    if (type == "enum") {
        result["values"] = json_enum_values(option, ch, lang);

    } else if (type == "account") {
        // Not typed into: the player links the character elsewhere and the
        // dialog offers what can be done to that link from here.
        result["actions"] = json_enum_values(option, ch, lang);

    } else if (type == "int") {
        result["min"] = option->getMinValue();
        result["max"] = option->getMaxValue();
        if (option->getStep() > 0)
            result["step"] = option->getStep();
        result["offValue"] = option->getOffValue();

    } else {
        if (!option->getPlaceholder().empty())
            result["placeholder"] = option->getPlaceholder().c_str();
        if (!option->getPattern().empty())
            result["pattern"] = option->getPattern().c_str();
    }

    return result;
}

/*-------------------------------------------------------------------------
 * Schema: every setting, on the page the world puts it on
 *------------------------------------------------------------------------*/

/** Options are gathered per page first and sorted afterwards, because the order
 *  they are declared in is the order of the text command -- one column, read
 *  top to bottom -- and not the order a dialog wants. */
struct PageOptions {
    std::vector<std::pair<int, Json::Value> > options;

    Json::Value toJson() const
    {
        std::vector<std::pair<int, Json::Value> > sorted = options;
        std::stable_sort(sorted.begin(), sorted.end(),
                         [](const std::pair<int, Json::Value> &a,
                            const std::pair<int, Json::Value> &b) {
                             return a.first < b.first;
                         });

        Json::Value result;
        for (auto &option: sorted)
            result.append(option.second);

        return result;
    }
};

/** The dialog as it can be built with no help from the world: one section, and
 *  a page per heading of the text command. The options that hold a value join
 *  the last page, which is where the command itself prints them. */
static Json::Value schema_from_text_groups(ConfigCommand *config, PCharacter *ch,
                                           lang_t lang, Json::Value &schema)
{
    Json::Value section;
    section["key"] = "server";
    section["label"] = config->getNameFor(lang).c_str();

    // An index rather than a pointer into the array: what a json value does to
    // the things already in it when another is appended is its own business.
    int last = -1;
    int number = 0;

    for (auto &textGroup: config->getGroups()) {
        Json::Value page;
        page["key"] = ("group" + DLString(++number)).c_str();
        page["label"] = json_text(textGroup.name, lang).c_str();
        page["search"] = search_words(textGroup.name).c_str();

        for (auto &element: textGroup)
            if (element->available(ch))
                page["options"].append(json_flag_option(*element, ch, lang));

        if (!page.isMember("options"))
            continue;

        section["pages"].append(page);
        last = section["pages"].size() - 1;
    }

    // The options that hold a value have no heading of their own in the text
    // command either: it prints them after the last one.
    if (last < 0) {
        if (!config->getValueElements().empty())
            LogStream::sendWarning()
                << "config schema: no headings to put the value options on" << endl;
    } else {
        for (auto &element: config->getValueElements())
            if (element.available(ch))
                section["pages"][last]["options"].append(
                    json_value_option(&element, ch, lang));
    }

    if (section.isMember("pages"))
        schema["sections"].append(section);

    return schema;
}

static Json::Value config_schema_json(PCharacter *ch)
{
    ConfigCommand *config = ConfigCommand::getThis();
    Json::Value schema;

    if (!config)
        return schema;

    lang_t lang = Player::displayLang(ch);
    std::map<DLString, PageOptions> pages;

    for (auto &textGroup: config->getGroups())
        for (auto &element: textGroup)
            if (element->available(ch))
                pages[element->getWebPage()].options.push_back(
                    std::make_pair(element->getWebOrder(),
                                   json_flag_option(*element, ch, lang)));

    for (auto &element: config->getValueElements())
        if (element.available(ch))
            pages[element.getWebPage()].options.push_back(
                std::make_pair(element.getWebOrder(),
                               json_value_option(&element, ch, lang)));

    schema["lang"] = lang2attr(lang).c_str();
    // The command that does the same thing at the keyboard: the dialog shows it
    // so the player learns the terminal instead of being walled off from it.
    schema["command"] = config->getNameFor(lang).c_str();

    // An older copy of the world files describes no dialog at all: no sections,
    // no pages, every option naming a page that does not exist. Rather than
    // hand the client an empty tree, fall back to the headings the text command
    // already prints -- the dialog comes out plainer, but whole, and the two
    // repositories can be merged in either order.
    if (config->getWebPages().empty())
        return schema_from_text_groups(config, ch, lang, schema);

    // The tree is laid out the way the world declares it; a page with nothing
    // on it is left out rather than shown empty.
    std::set<DLString> known;

    for (auto &section: config->getWebSections()) {
        Json::Value sectionJson;
        sectionJson["key"] = section.getKey().c_str();
        sectionJson["label"] = json_text(section.getName(), lang).c_str();

        for (auto &page: config->getWebPages()) {
            if (page.getSection() != section.getKey())
                continue;

            known.insert(page.getKey());

            std::map<DLString, PageOptions>::iterator found
                = pages.find(page.getKey());
            if (found == pages.end())
                continue;

            Json::Value pageJson;
            pageJson["key"] = page.getKey().c_str();
            pageJson["label"] = json_text(page.getName(), lang).c_str();
            pageJson["search"] = search_words(page.getName()).c_str();

            DLString subtitle = plain_text(json_text(page.getSubtitle(), lang), ch);
            if (!subtitle.empty())
                pageJson["subtitle"] = subtitle.c_str();

            pageJson["options"] = found->second.toJson();
            sectionJson["pages"].append(pageJson);
        }

        if (sectionJson.isMember("pages"))
            schema["sections"].append(sectionJson);
    }

    // An option naming a page that nobody declared would simply vanish from the
    // dialog, and a typo in the world file is a long thing to look for in
    // silence.
    for (auto &page: pages)
        if (known.count(page.first) == 0)
            LogStream::sendWarning()
                << "config schema: no page '" << page.first << "' for "
                << page.second.options.size() << " option(s)" << endl;

    return schema;
}

/*-------------------------------------------------------------------------
 * Values: what every setting is right now
 *------------------------------------------------------------------------*/
static Json::Value config_values_json(PCharacter *ch)
{
    ConfigCommand *config = ConfigCommand::getThis();
    Json::Value values;

    if (!config)
        return values;

    for (auto &textGroup: config->getGroups())
        for (auto &element: textGroup)
            if (element->available(ch))
                values[element->getName().c_str()] = element->isSetBit(ch);

    config_value_json(ch, values);
    return values;
}

void ConfigWebPromptListener::run( Descriptor *d, Character *ch, Json::Value &json )
{
    PCharacter *pch = ch->getPC();
    if (!pch)
        return;

    WebPromptAttribute::Pointer attr
        = pch->getAttributes( ).getAttr<WebPromptAttribute>( "webprompt" );
    Json::Value &prompt = json["args"][0];

    attr->updateIfNew( CONFIG, config_values_json( pch ), prompt );
}

/*-------------------------------------------------------------------------
 * The two calls the dialog makes
 *------------------------------------------------------------------------*/

/** Is this character actually in the world? Between the greeting and the first
 *  step there is a character already -- the one the entry script is filling in
 *  -- and its settings are nobody's yet: answering then would show the player a
 *  dialog full of switches that belong to no one. */
static bool playing(Character *ch)
{
    return ch->desc && ch->desc->connected == CON_PLAYING;
}

/** Everything the dialog needs to draw itself. Asked once, when it first opens;
 *  a client that gets no answer knows this server has no settings to offer. */
RPCRUN(config_schema)
{
    PCharacter *pch = ch->getPC();
    if (!pch || !playing(ch))
        return;

    Json::Value response;
    response["command"] = "config_schema";
    response["args"][0] = config_schema_json(pch);

    ch->desc->writeWSCommand(response);
}

/** Change one setting. Everything past this point is the code the 'config'
 *  command runs, down to the line it prints in reply, so a setting behaves the
 *  same whether it was clicked or typed. */
RPCRUN(config_set)
{
    PCharacter *pch = ch->getPC();
    if (!pch || !playing(ch))
        return;

    if (args.size() < 2) {
        LogStream::sendWarning() << "config_set: not enough arguments: " << args.size() << endl;
        return;
    }

    ConfigCommand *config = ConfigCommand::getThis();
    if (!config)
        return;

    const DLString &key = args[0];
    const DLString &value = args[1];

    for (auto &textGroup: config->getGroups())
        for (auto &element: textGroup) {
            if (element->getName() != key)
                continue;

            if (!element->available(pch))
                return;

            if (!element->handleArgument(pch, value))
                pch->pecho(_("Неправильный переключатель. См. {W? режим{x."));

            return;
        }

    for (auto &element: config->getValueElements()) {
        if (element.getName() != key)
            continue;

        if (!element.available(pch))
            return;

        // An empty box means the player wants the value gone. Typed into the
        // terminal an empty argument means 'tell me what it is now', which is
        // no use to a dialog that can see the value already.
        bool done = (value.empty() && element.getWebType() == "string")
                        ? config_value_run(pch, key, "clear")
                        : config_value_run(pch, key, value);

        // Described in the world but unknown to the code: the dialog would show
        // the setting and quietly do nothing with it.
        if (!done)
            LogStream::sendWarning()
                << "config_set: value option '" << key << "' has no handler" << endl;

        return;
    }

    LogStream::sendWarning() << "config_set: unknown option " << key << endl;
}

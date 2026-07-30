/* Display names of the browsable help categories -- HELP_IA.md. */
#include "helpcategory.h"

static const struct {
    const char *key;
    const char *name[3];        // indexed by the slot below: RU, EN, UA
} CATEGORY_NAME[] = {
    { "start",   { "С чего начать",  "Getting started",  "З чого почати" } },
    { "char",    { "Твой персонаж",  "Your character",   "Твій персонаж" } },
    { "combat",  { "Бой",            "Combat",           "Бій" } },
    { "skills",  { "Умения и обучение", "Skills and learning", "Вміння й навчання" } },
    { "magic",   { "Заклинания и магия", "Spells and magic", "Закляття й магія" } },
    { "classes", { "Классы",         "Classes",          "Класи" } },
    { "races",   { "Расы",           "Races",            "Раси" } },
    { "gods",    { "Боги и религии", "Gods and religions", "Боги й релігії" } },
    { "items",   { "Вещи и хозяйство", "Items and economy", "Речі й господарство" } },
    { "quests",  { "Квесты",         "Quests",           "Квести" } },
    { "world",   { "Мир и путешествия", "World and travel", "Світ і подорожі" } },
    { "society", { "Игроки и кланы", "Players and clans",  "Гравці й клани" } },
    { "comm",    { "Общение и настройки", "Communication and settings",
                   "Спілкування й налаштування" } },
    { "socials", { "Социалы",        "Socials",          "Соціали" } },
    { NULL,      { NULL, NULL, NULL } }
};

/** The word that follows the command name: 'help index', 'справка разделы'. */
static const char * INDEX_KEYWORD[3] = { "разделы", "index", "розділи" };

static int lang_slot(lang_t lang)
{
    switch (lang) {
        case EN: return 1;
        case UA: return 2;
        default: return 0;
    }
}

const char * help_index_keyword(lang_t lang)
{
    return INDEX_KEYWORD[lang_slot(lang)];
}

DLString help_category_name(const DLString &key, lang_t lang)
{
    for (int i = 0; CATEGORY_NAME[i].key; i++)
        if (key == CATEGORY_NAME[i].key)
            return CATEGORY_NAME[i].name[lang_slot(lang)];

    return key;
}

DLString help_category_from_argument(const DLString &arg)
{
    DLString needle = arg;
    needle.toLower();

    for (int i = 0; CATEGORY_NAME[i].key; i++) {
        DLString key = CATEGORY_NAME[i].key;
        key.toLower();
        if (needle == key)
            return CATEGORY_NAME[i].key;

        // Accept the display name in any language, not only the viewer's: a
        // player who copies a category name out of a friend's paste should not
        // be told it does not exist.
        for (int l = 0; l < 3; l++) {
            DLString name = CATEGORY_NAME[i].name[l];
            name.toLower();
            if (needle == name)
                return CATEGORY_NAME[i].key;
        }
    }

    return DLString::emptyString;
}

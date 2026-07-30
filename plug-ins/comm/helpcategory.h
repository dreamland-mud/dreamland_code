/* Display names of the browsable help categories -- HELP_IA.md.
 *
 * Shared by the 'help index' browser and by the 'commands' table, which group
 * by exactly the same 14 categories: the keys come from
 * HelpArticle::playerCategories() and the names from here, so the two views
 * cannot drift apart.
 */
#ifndef HELPCATEGORY_H
#define HELPCATEGORY_H

#include "lang.h"
#include "dlstring.h"

/** Display name of a browsable category in the viewer's language, or the bare
 *  key when it names no known category. */
DLString help_category_name(const DLString &key, lang_t lang);

/** Match what the player typed against a category: its key or its display name
 *  in any language. Empty string when nothing matches. */
DLString help_category_from_argument(const DLString &arg);

/** The word that follows the command name: 'help index', 'справка разделы'. */
const char * help_index_keyword(lang_t lang);

#endif

/* Automatic "see also" for help articles -- help revamp Phase 2 task 4.
 *
 * Every article ends with up to four related articles: the hand-picked ids of
 * its 'seealso' attribute first, then articles of the same category that link
 * to it, then articles of the same category sharing a rare keyword, then
 * articles of other categories that link to it. Nothing the body already links
 * to is repeated. scripts/help-related.py is the specification and the offline
 * test bench: a change to the ranking goes there first, then here.
 */
#ifndef HELPRELATED_H
#define HELPRELATED_H

#include <vector>
#include "helpmanager.h"

class Character;

/** Up to 'cap' related articles 'ch' may see, strongest first. */
std::vector<HelpArticle::Pointer> help_related(const HelpArticle *article, Character *ch, unsigned int cap = 4);

/** The "See also" line for the end of an article, or an empty string. */
DLString help_related_line(const HelpArticle *article, Character *ch);

#endif

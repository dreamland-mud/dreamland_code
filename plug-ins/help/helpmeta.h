/* The metadata block a help article heads with: one bullet per fact about the
 * thing the article describes -- a zone's levels and author, a race's stats,
 * a religion's availability.
 *
 * The pad is the one skill helps have always used (SKILL_INFO_PAD); it lives
 * here as well so the article types cannot drift apart on the separator or the
 * colouring, and because a run of these lines is exactly what the website's
 * renderer turns into a real <ul> -- a list a screen reader announces as one,
 * rather than the hand-aligned colon columns it replaces (those only ever
 * lined up in Russian anyway).
 */
#ifndef HELPMETA_H
#define HELPMETA_H

#include "dlstring.h"

extern const char *HELP_META_PAD;

/** One metadata bullet: "  * Label: value", no trailing newline. */
DLString help_meta_line(const DLString &label, const DLString &value);

#endif

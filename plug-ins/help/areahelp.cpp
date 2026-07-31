/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>
#include "areahelp.h"
#include "helpmeta.h"
#include "areabehaviorplugin.h"
#include "regexp.h"
#include "character.h"
#include "merc.h"
#include "string_utils.h"
#include "dl_strings.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

/*-------------------------------------------------------------------
 * AreaHelp 
 *------------------------------------------------------------------*/
const DLString AreaHelp::TYPE = "AreaHelp";

void AreaHelp::setAreaIndex(AreaIndexData *pArea)
{
    DLString aname = pArea->name.get(RU).ruscase('1').colourStrip();

    areafile = pArea->area_file;
    
    addAutoKeyword(String::toNormalizedList(pArea->name));
    addAutoKeyword(String::toNormalizedList(pArea->altname));
    refreshKeywords();

    // Quick check to distinguish help about this area from helps about other topics
    // TODO: mark such help with a non-transient 'area' label?
    selfHelp = is_name(aname.c_str(), keyword.get(RU).c_str());

    if (selfHelp) 
        labels.addTransient("area");
}

void AreaHelp::save() const
{
    if (areafile)
        areafile->area->changed = true;
}

DLString AreaHelp::getTitle(const DLString &label) const
{
    ostringstream buf;
    AreaIndexData *area = areafile->area;
    DLString t = title.get(RU);

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (!t.empty())
            return t;
            
        buf << "Зона '" << area->getName() << "'";
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return t;
    }

    if (!t.empty() || !selfHelp)
        return MarkupHelpArticle::getTitle(label);

    buf << "Зона {c" << area->getName() << "{x";

    return buf.str();
}

/**
 * Same title, composed in the viewer's language.
 *
 * Only ~220 of the 1300-odd articles carry an authored <title>; the rest have
 * theirs assembled here at display time, and that assembly used to be Russian
 * for every viewer -- including the "toc" form the website's category rail is
 * built from. The frame word comes from the catalog, the zone name from the
 * area's own per-language name.
 */
DLString AreaHelp::getTitle(const DLString &label, lang_t lang) const
{
    // An authored <title> already resolves per language in the base, and a
    // help with no area has nothing to compose from.
    if (!title.get(RU).empty() || !areafile || !areafile->area)
        return MarkupHelpArticle::getTitle(label, lang);

    // Website: the article's own <h1> -- the page supplies its own heading.
    if (label == "title")
        return DLString::emptyString;

    DLString name = areafile->area->getName(lang);

    // Website: right-hand side table of contents
    if (label == "toc")
        return name.upperFirstCharacter();

    if (!selfHelp)
        return MarkupHelpArticle::getTitle(label, lang);

    return help_title_fmt(lang, _("Зона {c%1$s{x"), name);
}

static void format_area_quest(AreaQuest *q, ostringstream &qbuf, Character *ch)
{
    // %A% is a website map macro -- keep it out of any fmt() format string.
    qbuf << "{Y%A%{x " << q->description.getForLang(viewerLang(ch))
            << "  " << _("Уровни: ").getMessage(ch);

    // Restrictions by level and align
    if (q->minLevel > 0 && q->maxLevel < LEVEL_MORTAL)
        qbuf << fmt(ch, _("с %1$d по %2$d. "), q->minLevel.getValue(), q->maxLevel.getValue());
    else if (q->maxLevel < LEVEL_MORTAL)
        qbuf << fmt(ch, _("до %1$d. "), q->maxLevel.getValue());
    else if (q->minLevel > 0)
        qbuf << fmt(ch, _("с %1$d. "), q->minLevel.getValue());
    else
        qbuf << _("любые. ").getMessage(ch);

    // align noun stays RU (the align table is not externalized); localize the frame.
    if (q->align.getValue() != 0)
        qbuf << _("Натура: ").getMessage(ch) << q->align.messages(true, '1', viewerLang(ch)) << ". ";

    // Quest frequency
    qbuf << _("Как часто: ").getMessage(ch);
    if (q->limitPerLife > 0)
        qbuf << fmt(ch, _("%1$d раз%1$I|а| за жизнь"), q->limitPerLife.getValue());
    else // TODO support for 'once per hour' etc
        qbuf << _("сколько угодно раз").getMessage(ch);

    qbuf << "." << endl;
}

void AreaHelp::getRawText( Character *ch, ostringstream &in ) const
{
    AreaIndexData *area = areafile->area;

    if (!selfHelp) {
        MarkupHelpArticle::getRawText(ch, in);
        return;
    }

    lang_t lang = viewerLang(ch);

    /* The website and the maps page put the zone name in the article's own
     * <h1>, so for the json dump (no descriptor) the heading would be the same
     * words twice over. In game there is no <h1> to inherit from. */
    if (ch && ch->desc)
        in << fmt(ch, _("Зона {c%1$s{x"), area->getName(lang).c_str()) << endl;

    /* Everything the article knows ABOUT the zone, one bullet each. It used to
     * be a run-on first line, a stray line for the danger level, and the way in
     * at the very bottom, below the article text. */
    if (area->low_range > 0 || area->high_range > 0) {
        ostringstream levels;

        levels << "{Y" << area->low_range << "-" << area->high_range << "{x";
        in << help_meta_line(l(ch, "Уровни"), levels.str()) << endl;
    }

    if (IS_SET(area->area_flag, AREA_SAFE|AREA_EASY|AREA_HARD|AREA_DEADLY))
        in << help_meta_line(l(ch, "Опасность"), area_danger_long(area, ch)) << endl;

    // Every zone has one today, but a bullet with nothing after the colon is a
    // visible defect in a way a trailing "author " never was.
    if (!area->authors.empty())
        in << help_meta_line(l(ch, "Автор"), DLString("{y") + area->authors + "{x") << endl;

    if (!area->translator.empty())
        in << help_meta_line(l(ch, "Перевод"), DLString("{y") + area->translator + "{x") << endl;

    /* The alternative name in the VIEWER's language, and only that one: the
     * list used to be assembled from every language at once with just the
     * Russian main name taken back out, so an English reader was shown their
     * own zone name a second time and the Ukrainian one after it. Strict
     * get() rather than getForLang() -- an alias is a garnish, and falling
     * back would drop an English word into a Russian line. */
    DLString altname = area->altname.get(lang).ruscase('1').colourStrip();

    if (!altname.empty() && altname != area->getName(lang).colourStrip())
        in << help_meta_line(l(ch, "Также известна как"), altname) << endl;

    if (!area->speedwalk.emptyValues()) {
        // Was LANG_DEFAULT: a zone whose way in is prose rather than a run path
        // read Russian to everybody.
        const DLString &speedwalk = area->speedwalk.getForLang(lang);
        ostringstream way;

        // For speedwalks that only contain run path, surround it with {hs tags.
        RegExp simpleSpeedwalkRE("^[0-9nsewud]+$");
        if (simpleSpeedwalkRE.match(speedwalk))
            way << "{y{hs" << speedwalk << "{x";
        else
            way << speedwalk;

        // A path spelled in directions has to start somewhere; prose says so itself.
        RegExp speedwalkRE("[0-9]?[nsewud]+");
        if (speedwalkRE.match(speedwalk))
            way << " {D" << l(ch, "(от Рыночной Площади)") << "{x";

        in << help_meta_line(l(ch, "Как добраться"), way.str()) << endl;
    }

    /* Web clients turn the marker into a link to the zone's map page. Telnet
     * has nothing to open, so the whole bullet -- its newline included -- sits
     * inside the web-only branch, and %PAUSE% keeps the [brackets] from being
     * read as a help reference. */
    in << "%PAUSE%{Iw"
       << help_meta_line(l(ch, "Карта"), DLString("[map=") + areafile->file_name + "]")
       << endl << "{Ix%RESUME%";

    in << endl;

    if (!text.getForLang(lang).empty())
       in << text.getForLang(lang) << endl;

    if (!area->quests.empty()) {
        ostringstream qbuf;

        for (auto &q: area->quests)
            if (!q->flags.isSet(AQUEST_HIDDEN))
                format_area_quest(*q, qbuf, ch);

        if (!qbuf.str().empty())
            in << _("{yЗадания{x:").getMessage(ch) << endl << qbuf.str() << endl;
    }
}

/** Get self-help article for this area. */
AreaHelp * area_selfhelp(AreaIndexData *area)
{
    for (auto &article: area->helps) {
        AreaHelp *ahelp = article.getDynamicPointer<AreaHelp>();
        if (ahelp && ahelp->selfHelp)
            return ahelp;
    }

    return 0;
}

int area_helpid(struct AreaIndexData *area)
{
    AreaHelp *ahelp = area_selfhelp(area);
    int hid = ahelp && !help_is_empty(*ahelp) ? ahelp->getID() : -1;
    return hid;
}


/** Return true if this article is empty or consists only of spaces. */
bool help_is_empty(const HelpArticle &help)
{
    return help.text.get(RU).find_first_not_of(' ') == DLString::npos;
}

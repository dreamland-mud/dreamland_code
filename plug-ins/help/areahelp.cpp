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

/* No central place holds the site address -- gmcp/impl.cpp keeps its own and
 * messengers.cpp inlines one -- so this follows suit. */
static const DLString MAPS_URL = "https://dreamland.rocks/maps.html#";

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
     * at the very bottom, below the article text.
     *
     * The whole block is %PAUSE%d, the way skill and language helps have always
     * fenced theirs. The help formatter owns `*`, `_`, `=`, `(...)` and `[...]`
     * as markup, so an unfenced block loses the bullet itself (`*...*` is its
     * bold) and any parenthesised aside (`(eng,rus)` is its language choice). */
    in << "%PAUSE%";

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
        bool runPath = simpleSpeedwalkRE.match(speedwalk);

        if (runPath)
            way << "{y{hs" << speedwalk << "{x";
        else
            way << speedwalk;

        /* A bare run of directions has to start somewhere. Only for a bare one:
         * where the field is prose it names its own starting point already, and
         * the note used to be appended to those too ("4e3n2wn from the Market
         * Square (from the Market Square)"). */
        if (runPath)
            way << " {D" << l(ch, "(от Рыночной Площади)") << "{x";

        in << help_meta_line(l(ch, "Как добраться"), way.str()) << endl;
    }

    /* One bullet, two payloads, and the gates wrap ONLY the payloads.
     *
     * Wrapping the whole line does not work, which is how this was written and
     * why it was broken: `{x` resets st_invis to INVIS_NONE, and the bullet pad
     * in help_meta_line carries a `{x` five characters in, right behind the
     * star. The gate sprang open at the star and everything after it -- label,
     * colon, payload -- went to every reader regardless. Telnet has been shown a
     * bare "Карта: [map=newthalos.are]" all along, minus its star, and the same
     * half-open gate left an unbalanced raw-region marker in the zone-help JSON
     * the website serves.
     *
     * Neither span below contains a `{x`, so neither springs, and `{Ix` now
     * arrives with st_invis intact and writes its closing marker.
     *
     * Both halves point at the same page. maps.html#<zone> resolves for all 156
     * zones -- its script reads location.hash on load, so it works from a cold
     * paste -- while the per-zone /maps/<zone>.html pages are the pre-redesign
     * site and 404 for twenty of them. That page also carries the text view
     * screen readers need, which the ASCII-art one does not.
     *
     * file_name is copied before replaces(): it mutates in place and returns a
     * reference, so calling it on the area's own field would rewrite the stored
     * filename for the rest of the run. */
    DLString mapZone = areafile->file_name;
    ostringstream mapValue;

    mapValue << "{Iw[map=" << areafile->file_name << "]{Ix"
             << "{IW" << MAPS_URL << mapZone.replaces(".are", "") << "{Ix";

    in << help_meta_line(l(ch, "Карта"), mapValue.str()) << endl;

    in << "%RESUME%" << endl;

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

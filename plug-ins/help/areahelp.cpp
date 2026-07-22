/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>
#include "areahelp.h"
#include "areabehaviorplugin.h"
#include "regexp.h"
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

    in << fmt(ch, _("Зона {Y%1$s{x, "), area->getName(viewerLang(ch)).c_str());

    if (area->low_range > 0 || area->high_range > 0)
       in << fmt(ch, _("уровни {Y%1$d-%2$d{x, "), area->low_range, area->high_range);

    in << fmt(ch, _("автор {y%1$s{x"), area->authors.c_str());

    if (!area->translator.empty())
        in << fmt(ch, _(", перевод {y%1$s{x"), area->translator.c_str());

    // This bit is going to be replaced with a link to the map by the webclient.
    in << "%PAUSE% {Iw[map=" << areafile->file_name << "]{Ix%RESUME%";

    in << endl;

    // Make a list of all alternative names excluding the main one.
    list<DLString> altnames = String::toNormalizedList(area->altname);
    list<DLString> names = String::toNormalizedList(area->name);
    names.splice(names.end(), altnames);
    names.remove(area->getName().colourStrip());

    if (!names.empty())
        in << fmt(ch, _("{DТакже известна как: %1$s{x"), String::join(names, ", ").c_str()) << endl;

    if (IS_SET(area->area_flag, AREA_SAFE|AREA_EASY|AREA_HARD|AREA_DEADLY))
        in << _("Уровень опасности: ").getMessage(ch) << area_danger_long(area, ch) << endl;

    in << endl;

    if (!text.getForLang(viewerLang(ch)).empty())
       in << text.getForLang(viewerLang(ch)) << endl;

    if (!area->quests.empty()) {
        ostringstream qbuf;

        for (auto &q: area->quests)
            if (!q->flags.isSet(AQUEST_HIDDEN))
                format_area_quest(*q, qbuf, ch);

        if (!qbuf.str().empty())
            in << _("{yЗадания{x:").getMessage(ch) << endl << qbuf.str() << endl;
    }

    if (!area->speedwalk.emptyValues()) {
        const DLString &speedwalk = area->speedwalk.get(LANG_DEFAULT);

        in << _("{yКак добраться{x: ").getMessage(ch);

        // For speedwalks that only contain run path, surround it with {hs tags.
        RegExp simpleSpeedwalkRE("^[0-9nsewud]+$");
        if (simpleSpeedwalkRE.match(speedwalk))
            in << "{y{hs" << speedwalk << "{x" << endl;
        else
            in << speedwalk << endl;

        // If 'speedwalk' field contains something resembling a run path,
        // and not just text, explain the starting point.
        RegExp speedwalkRE("[0-9]?[nsewud]+");
        if (speedwalkRE.match(speedwalk))
           in << _("{D(все пути ведут от Рыночной Площади Мидгаарда, если не указано иначе){x").getMessage(ch) << endl;
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

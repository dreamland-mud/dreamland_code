/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>
#include "areahelp.h"
#include "areabehaviorplugin.h"
#include "regexp.h"
#include "merc.h"

#include "dl_strings.h"
#include "act.h"
#include "def.h"

/*-------------------------------------------------------------------
 * AreaHelp 
 *------------------------------------------------------------------*/
const DLString AreaHelp::TYPE = "AreaHelp";

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

    if (strlen(area->credits) > 0 
            && str_str(area->credits, area->getName().c_str()) == 0
            && str_str(area->getName().c_str(), area->credits) == 0)
        buf << " ({c" << area->credits << "{x)";

    return buf.str();
}

static void format_area_quest(AreaQuest *q, ostringstream &qbuf)
{
    qbuf << "{Y%A%{x " << q->description
            << "  " << "Уровни: ";

    // Restrictions by level and align
    if (q->minLevel > 0 && q->maxLevel < LEVEL_MORTAL)
        qbuf << "с " << q->minLevel << " по " << q->maxLevel << ". ";
    else if (q->maxLevel < LEVEL_MORTAL)
        qbuf << "до " << q->maxLevel << ". ";
    else if (q->minLevel > 0)
        qbuf << "с " << q->minLevel << ". ";
    else
        qbuf << "любые. ";

    if (q->align.getValue() != 0)
        qbuf << "Натура: " << q->align.messages(true, '1') << ". ";

    // Quest frequency
    qbuf << "Как часто: ";
    if (q->limitPerLife > 0)
        qbuf << fmt(0, "%1$d раз%1$I|а| за жизнь", q->limitPerLife.getValue());
    else // TODO support for 'once per hour' etc
        qbuf << "сколько угодно раз";

    qbuf << "." << endl;
}

void AreaHelp::getRawText( Character *ch, ostringstream &in ) const
{
    AreaIndexData *area = areafile->area;
    
    if (!selfHelp) {
        MarkupHelpArticle::getRawText(ch, in);
        return;
    }

    in << "Зона {Y" << area->getName() << "{x, ";

    if (area->low_range > 0 || area->high_range > 0)
       in << "уровни {Y" << area->low_range << "-" << area->high_range << "{x, ";

    in << "автор {y" << area->authors << "{x";

    if (str_cmp(area->translator, ""))
        in << ", перевод {y" << area->translator << "{x";

    // This bit is going to be replaced with a link to the map by the webclient.
    in << "%PAUSE% {Iw[map=" << areafile->file_name << "]{Ix%RESUME%";

    in << endl;

    if (IS_SET(area->area_flag, AREA_SAFE|AREA_EASY|AREA_HARD|AREA_DEADLY))
        in << "Уровень опасности: " << area_danger_long(area) << endl;

    in << endl;

    if (!text.get(RU).empty())
       in << text.get(RU) << endl;

    if (!area->quests.empty()) {
        ostringstream qbuf;

        for (auto &q: area->quests)
            if (!q->flags.isSet(AQUEST_HIDDEN))
                format_area_quest(*q, qbuf);

        if (!qbuf.str().empty())
            in << "{yЗадания{x:" << endl << qbuf.str() << endl;
    }

    if (str_cmp(area->speedwalk, "")) {
        in << "{yКак добраться{x: ";

        // For speedwalks that only contain run path, surround it with {hs tags.
        RegExp simpleSpeedwalkRE("^[0-9nsewud]+$");
        if (simpleSpeedwalkRE.match(area->speedwalk))
            in << "{y{hs" << area->speedwalk << "{x" << endl;
        else
            in << area->speedwalk << endl;

        // If 'speedwalk' field contains something resembling a run path,
        // and not just text, explain the starting point.
        RegExp speedwalkRE("[0-9]?[nsewud]+");
        if (speedwalkRE.match(area->speedwalk))
           in << "{D(все пути ведут от Рыночной Площади Мидгаарда, если не указано иначе){x" << endl;
    }
}

/** Get self-help article for this area, either a real one or automatically created. */
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

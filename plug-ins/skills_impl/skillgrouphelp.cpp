/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "skillmanager.h"
#include "skillgrouphelp.h"
#include "character.h"
#include "xmltableelement.h"
#include "l10n.h"

/*-------------------------------------------------------------------
 * SkillGroupHelp 
 *------------------------------------------------------------------*/
const DLString SkillGroupHelp::TYPE = "SkillGroupHelp";

void SkillGroupHelp::save() const
{
    if (group) {
        const XMLTableElement *element = group.getDynamicPointer<XMLTableElement>();
        if (element)
            element->save();
    }
}

DLString SkillGroupHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        if (group)
            buf << group->getRussianName().upperFirstCharacter();
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }
    
    if (!group || !title.get(RU).empty())
        return MarkupHelpArticle::getTitle(label);

    return "Группа умений {c" + group->getRussianName() + "{x";
}

/**
 * Same title, composed in the viewer's language.
 *
 * Most articles carry no authored <title> -- theirs is assembled here at
 * display time, and that assembly was Russian for every viewer, including the
 * "toc" form the website's category rail is built from.
 */
DLString SkillGroupHelp::getTitle(const DLString &label, lang_t lang) const
{
    if (!group || !title.get(RU).empty())
        return MarkupHelpArticle::getTitle(label, lang);

    if (label == "title")
        return DLString::emptyString;

    DLString name = group->getNameFor(lang);

    if (label == "toc")
        return name.upperFirstCharacter();

    return help_title_fmt(lang, _("Группа умений {c%1$s{x"), name);
}

void SkillGroupHelp::getRawText( Character *ch, ostringstream &buf ) const
{
    buf << "%PAUSE%";
    group->show( ch->getPC( ), buf );
}

void SkillGroupHelp::setSkillGroup( SkillGroup::Pointer group )
{
    this->group = group;
    
    addAutoKeyword( group->getName( ) );    
    addAutoKeyword( group->getRussianName( ) );    
    addAutoKeyword( group->getNameFor( UA ) );    
    labels.addTransient("skillgroup");
    helpManager->registrate( Pointer( this ) );
}

void SkillGroupHelp::unsetSkillGroup( )
{
    helpManager->unregistrate( Pointer( this ) );
    group.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}


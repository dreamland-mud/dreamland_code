/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>
#include <set>

#include "logstream.h"
#include "skill_utils.h"
#include "skillhelp.h"    
#include "skill.h"
#include "spell.h"
#include "command.h"
#include "xmltableelement.h"
#include "skillcommand.h"
#include "skillgroup.h"
#include "character.h"
#include "player_utils.h"
#include "commandflags.h"
#include "def.h"
#include "l10n.h"

GROUP(none);
const DLString SkillHelp::TYPE = "SkillHelp";

void SkillHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "%PAUSE%";
    skill->show(ch->getPC(), in);
    in << "%RESUME%";

    in << endl
       << text.getForLang(Player::displayLang(ch));
}

SkillHelpFormatter::SkillHelpFormatter( const char *text, Skill::Pointer skill )
{
    this->text = text;
    this->skill = skill;
    this->cmd = skill->getCommand( );
    lang = LANG_DEFAULT;
}

SkillHelpFormatter::~SkillHelpFormatter( )
{
}

void SkillHelpFormatter::reset( )
{
    HelpFormatter::reset( );
    lang = LANG_DEFAULT;
}

void SkillHelpFormatter::setup( Character *ch )
{
    if (ch)
        lang = Player::displayLang(ch);

    HelpFormatter::setup( ch );
}


/*
 * CMD      ->  the command's name in the viewer's language
 * SKILL    ->  the skill's name in the viewer's language
 * SPELL    ->  к 'название заклинания'  (the cast command plus the spell)
 *
 * These used to be a two-way English-or-Russian choice, so a Ukrainian reader
 * got Russian everywhere. Both Command and Skill already carry getNameFor().
 *
 * The cast prefix stays a literal, because it is not a name but the shortest
 * thing you can TYPE: 'c' in English, 'к' in Russian. Ukrainian has no
 * one-letter form -- 'ч' also prefixes час and читати -- so it spells the
 * command out.
 */
bool SkillHelpFormatter::handleKeyword( const DLString &kw, ostringstream &out )
{
    if (HelpFormatter::handleKeyword( kw, out ))
        return true;
    
    if (kw == "CMD" && cmd) {
        out << cmd->getNameFor( lang );
        return true;
    }

    if (kw == "SKILL") {
        out << skill->getNameFor( lang ).quote( );
        return true;
    }

    if (kw == "SPELL") {
        out << l(viewer, "к") << " " << skill->getNameFor( lang ).quote( );
        return true;
    }
    
    return false;
}

void SkillHelp::save() const
{
    if (skill) {
        const XMLTableElement *element = skill.getDynamicPointer<XMLTableElement>();
        if (element)
            element->save();
    }
}

void SkillHelp::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    SkillHelpFormatter( in.str( ).c_str( ), 
                        skill 
                      ).run( ch, out );
}

/**
 * Return different help article title for web, depending on whether
 * we're displaying a list of skills or a list of commands.
 */
DLString SkillHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    if (!skill)
        return HelpArticle::getTitle(label);

    // Website: right-hand side table of contents
    if (label == "toc") {
        buf << skill_what(*skill).ruscase('1').upperFirstCharacter() << " '" << skill->getRussianName() << "'";
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    // Default title if not set explicitly.
    if (title.get(RU).empty()) {
        DLString title = (skill_is_spell(*skill) ? "Заклинание {c" : "Умение {c")
            + skill->getRussianName() + "{x";

        if (skill->getCommand() && !skill->getCommand()->getRussianName().empty())
            title += " и команда {c" 
                + skill->getCommand()->getRussianName() + "{x";

        return title;
    }

    return title.get(RU);
}

/**
 * Same title, composed in the viewer's language.
 *
 * Most articles carry no authored <title> -- theirs is assembled here at
 * display time, and that assembly was Russian for every viewer, including the
 * "toc" form the website's category rail is built from.
 */
DLString SkillHelp::getTitle(const DLString &label, lang_t lang) const
{
    if (!skill || !title.get(RU).empty())
        return HelpArticle::getTitle(label, lang);

    // Website: the article's own <h1> -- the page supplies its own heading.
    if (label == "title")
        return DLString::emptyString;

    DLString name = skill->getNameFor(lang);
    bool spell = skill_is_spell(*skill);

    // Website: right-hand side table of contents
    if (label == "toc")
        return spell
            ? help_title_fmt(lang, _("Заклинание '%1$s'"), name)
            : help_title_fmt(lang, _("Умение '%1$s'"), name);

    // In-game header. A skill that is also a command names both.
    if (skill->getCommand() && !skill->getCommand()->getNameFor(lang).empty()) {
        DLString cmd = skill->getCommand()->getNameFor(lang);
        return spell
            ? help_title_fmt(lang, _("Заклинание {c%1$s{x и команда {c%2$s{x"), name, cmd)
            : help_title_fmt(lang, _("Умение {c%1$s{x и команда {c%2$s{x"), name, cmd);
    }

    return spell
        ? help_title_fmt(lang, _("Заклинание {c%1$s{x"), name)
        : help_title_fmt(lang, _("Умение {c%1$s{x"), name);
}

void SkillHelp::setSkill( Skill::Pointer skill )
{
    this->skill = skill;
    
    addAutoKeyword( skill->getName( ) );    
    addAutoKeyword( skill->getRussianName( ) );    
    // The Ukrainian name was never added here, so `довідка вогняна куля` found
    // nothing even though the skill declares <name l="ua">. getNameFor falls
    // back to Russian, so a skill without one contributes a duplicate the
    // keyword set drops.
    addAutoKeyword( skill->getNameFor( UA ) );
    
    if (skill->getCommand( )) {
        Command::Pointer cmd = skill->getCommand( ).getDynamicPointer<Command>( );
        
        if (cmd) {
            addAutoKeyword(cmd->getName());
            addAutoKeyword(cmd->getNameFor(UA));
            addAutoKeyword(cmd->aliases.get(EN).split(" "));
            addAutoKeyword(cmd->aliases.get(RU).split(" "));
            addAutoKeyword(cmd->aliases.get(UA).split(" "));
            if (!cmd->getExtra().isSet(CMD_NO_INTERPRET)) {
                labels.addTransient("cmd");
            }
        }
    }
    
    if (skill->getSpell())
        labels.addTransient("spell");
    else
        labels.addTransient("skill");
        
    XMLVariableContainer *skillWithType = skill.getDynamicPointer<XMLVariableContainer>();
    if (skillWithType)
        labels.addTransient(skillWithType->getType().toLower());

    helpManager->registrate( Pointer( this ) );
}

void SkillHelp::unsetSkill( )
{
    helpManager->unregistrate( Pointer( this ) );
    skill.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}




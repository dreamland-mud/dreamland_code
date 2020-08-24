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
#include "commandflags.h"
#include "def.h"

GROUP(none);
const DLString SkillHelp::TYPE = "SkillHelp";

void SkillHelp::getRawText( Character *ch, ostringstream &in ) const
{
    bool rus = ch->getConfig( ).ruskills;

    in << skill_what(*skill).ruscase('1').upperFirstCharacter();
    if (rus)
        in << " '{c" << skill->getRussianName( ) << "{x' или" << " '{c" << skill->getName( ) << "{x'";
    else
        in << " '{c" << skill->getName( ) << "{x' или" << " '{c" << skill->getRussianName( ) << "{x'";

    SkillGroupReference &group = skill.getConstPointer<Skill>()->getGroup( );
    const DLString &gname = group->getNameFor(ch);
    if (group != group_none)
        in << ", входит в группу '{hg{c" << gname << "{x'";
    in << " " << editButton(ch) << endl;

    print_wait_and_mana(*skill, ch, in);

    in << endl
       << *this;
    
    // '... умение|slook herbs|травы'. - с гипер-ссылкой на команду
    // '... группаум|glist maladiction|проклятия' - с гипер-ссылкой на команду
    if (skill->visible(ch)) {
        in << "См. также команду {y{hc{lRумение{lEslook{lx " << skill->getNameFor(ch) << "{x";
        if (group != group_none)
        in << ", {y{hc{lRгруппаум{lEglist{lx " << gname << "{x";
        in << "." << endl;
    }
}

SkillHelpFormatter::SkillHelpFormatter( const char *text, Skill::Pointer skill )
{
    this->text = text;
    this->skill = skill;
    this->cmd = skill->getCommand( );
    fRusCmd = false;
    fRusSkill = false;
}

SkillHelpFormatter::~SkillHelpFormatter( )
{
}

void SkillHelpFormatter::reset( )
{
    HelpFormatter::reset( );
    fRusCmd = false;
    fRusSkill = false;
}

void SkillHelpFormatter::setup( Character *ch )
{
    if (ch) {
        PlayerConfig cfg = ch->getConfig( );

        fRusCmd = cfg.rucommands;
        fRusSkill = cfg.ruskills;
    }
    
    HelpFormatter::setup( ch );
}


/*
 * CMD      ->  {lEeng_name{lRрусское_имя{lx
 * SKILL    ->  {lEeng_name{lRрусское_имя{lx
 * SPELL    ->  {lEc 'spell name'{lRк 'название заклинания'{lx
 */
bool SkillHelpFormatter::handleKeyword( const DLString &kw, ostringstream &out )
{
    if (HelpFormatter::handleKeyword( kw, out ))
        return true;
    
    if (kw == "CMD" && cmd) {
        out << (fRusCmd && !cmd->getRussianName( ).empty( )
                        ? cmd->getRussianName( )
                        : cmd->getName( ));
        return true;
    }

    if (kw == "SKILL") {
        out << (fRusSkill ? skill->getRussianName( ).quote( )
                          : skill->getName( ).quote( ));
        return true;
    }

    if (kw == "SPELL") {
        out << (fRusCmd ? "к" : "c") << " "
            << (fRusSkill ? skill->getRussianName( ).quote( )
                          : skill->getName( ).quote( ));
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
    if (!skill)
        return HelpArticle::getTitle(label);

    // For help json dump.
    if (!label.empty()) {
        if (label == "cmd" && skill->getCommand()) 
            return skill->getCommand()->getName() + ", "
                    + skill->getCommand()->getRussianName();

        return skill->getName() + ", " + skill->getRussianName();
    }

    // Default title if not set explicitly.
    if (titleAttribute.empty()) {
        DLString title = (skill_is_spell(*skill) ? "Заклинание {c" : "Умение {c")
            + skill->getRussianName() + "{x, {c" + skill->getName() + "{x";

        if (skill->getCommand() && !skill->getCommand()->getRussianName().empty())
            title += " и команда {c" 
                + skill->getCommand()->getRussianName() + "{x, {c" 
                + skill->getCommand()->getName() + "{x";

        return title;
    }

    return titleAttribute;
}

void SkillHelp::setSkill( Skill::Pointer skill )
{
    this->skill = skill;
    
    addAutoKeyword( skill->getName( ) );    
    addAutoKeyword( skill->getRussianName( ) );    
    
    if (skill->getCommand( )) {
        Command::Pointer cmd = skill->getCommand( ).getDynamicPointer<Command>( );
        
        if (cmd) {
            addAutoKeyword( cmd->getName( ) );
            addAutoKeyword(cmd->getAliases().toSet());
            addAutoKeyword(cmd->getRussianAliases().toSet());
            if (!cmd->getExtra().isSet(CMD_NO_INTERPRET)) {
/*                
                labels.addTransient(
                    cmd->getCommandCategory().names());
*/                    
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




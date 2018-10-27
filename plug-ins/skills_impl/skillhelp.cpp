/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>
#include <set>

#include "skillhelp.h"    
#include "skill.h"
#include "spell.h"
#include "command.h"
#include "skillcommand.h"
#include "skillgroup.h"
#include "character.h"

const DLString SkillHelp::TYPE = "SkillHelp";

void SkillHelp::getRawText( Character *ch, ostringstream &in ) const
{
    bool rus = ch->getConfig( )->ruskills;

    in << (skill->getSpell( ) && skill->getSpell( )->isCasted( ) ? "Заклинание" : "Умение");
    if (rus)
        in << " '{c" << skill->getRussianName( ) << "{x' или" << " '{c" << skill->getName( ) << "{x'";
    else
        in << " '{c" << skill->getName( ) << "{x' или" << " '{c" << skill->getRussianName( ) << "{x'";

    SkillGroupReference &group = (const_cast<Skill *>(skill.getPointer( )))->getGroup( );
    in << ", входит в группу '{hg{c" 
       << (rus ? group->getRussianName( ) : group->getName( )) << "{x'"
       << endl << endl
       << *this;

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
        PlayerConfig::Pointer cfg = ch->getConfig( );

        fRusCmd = cfg->rucommands;
        fRusSkill = cfg->ruskills;
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


void SkillHelp::applyFormatter( Character *ch, ostringstream &in, ostringstream &out ) const
{
    SkillHelpFormatter( in.str( ).c_str( ), 
                        skill 
                      ).run( ch, out );
}

void SkillHelp::setSkill( Skill::Pointer skill )
{
    this->skill = skill;
    
    keywords.insert( skill->getName( ) );    
    keywords.insert( skill->getRussianName( ) );    
    
    if (!keyword.empty( ))
        keywords.fromString( keyword.toLower() );
    
    if (skill->getCommand( )) {
        Command::Pointer cmd = skill->getCommand( ).getDynamicPointer<Command>( );
        
        if (cmd) {
            keywords.insert( cmd->getName( ) );
            cmd->getAliases( ).toSet( keywords );
            cmd->getRussianAliases( ).toSet( keywords );
        }
    }
    
    fullKeyword = keywords.toString().toUpper();
    helpManager->registrate( Pointer( this ) );
}

void SkillHelp::unsetSkill( )
{
    helpManager->unregistrate( Pointer( this ) );
    skill.clear( );
    keywords.clear();
    fullKeyword = "";
}

bool SkillHelp::toXML( XMLNode::Pointer &parent ) const
{
    return XMLHelpArticle::toXML( parent ); 
}


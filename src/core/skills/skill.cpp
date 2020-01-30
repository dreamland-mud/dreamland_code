/* $Id$
 *
 * ruffina, 2004
 */
#include "russianstring.h"
#include "skill.h"
#include "spell.h"
#include "affecthandler.h"
#include "skillcommand.h"
#include "skilleventhandler.h"
#include "skillgroup.h"
#include "character.h"

GROUP(none);

Skill::Skill( )
{
}
Skill::Skill( const DLString &n ) : name( n )
{
}
Skill::~Skill( )
{
}
const DLString& Skill::getName( ) const
{
    return name;
}
bool Skill::isValid( ) const
{
    return false;
}
const DLString& Skill::getNameFor( Character *ch ) const
{
    if (ch->getConfig( )->ruskills)
        return getRussianName( );
    else
        return getName( );
}
const DLString &Skill::getRussianName( ) const
{
    return DLString::emptyString;
}
SkillGroupReference & Skill::getGroup( ) 
{
    return group_none;
}
Spell::Pointer Skill::getSpell( ) const
{
    return Spell::Pointer( );
}
AffectHandler::Pointer Skill::getAffect( ) const
{
    return AffectHandler::Pointer( );
}
SkillEventHandlerPointer Skill::getEventHandler( ) const
{
    return SkillEventHandler::Pointer( );
}
int Skill::getBeats( ) const
{
    return 0;
}
int Skill::getMana( ) const
{
    return 0;
}
const RussianString &Skill::getDammsg( ) const
{
    static RussianString dammsg("тупость");
    return dammsg;
}
bool Skill::visible( Character * ) const
{
    return false;
}
bool Skill::available( Character * ) const
{
    return false;
}
bool Skill::usable( Character *, bool message ) const  
{
    return false;
}
int Skill::getLevel( Character * ) const
{
    return 999;
}
int Skill::getLearned( Character * ) const
{
    return 0;
}
int Skill::getEffective( Character *ch ) const
{
    return getLearned( ch );
}
int Skill::getMaximum( Character * ) const
{
    return 0;
}
int Skill::getAdept( PCharacter * ) const
{
    return 0;
}
void Skill::improve( Character *, bool, Character *victim, int dam_type, int dam_flags ) const
{
}
const DLString & Skill::getCategory( ) const
{
    static DLString category = "under construction";
    return category;
}
bool Skill::canPractice( PCharacter *, std::ostream & ) const
{
    return false;
}
bool Skill::canTeach( NPCharacter *, PCharacter *, bool ) 
{
    return false;
}
void Skill::practice( PCharacter * ) const
{
}
void Skill::show( PCharacter *, std::ostream & ) 
{ 
}

class DummySkillCommand : public SkillCommand {
public:
    typedef ::Pointer<DummySkillCommand> Pointer;
    
    virtual void setSkill( SkillPointer ) { }
    virtual void unsetSkill( ) { }
    virtual SkillPointer getSkill( ) const { return SkillPointer( ); }
    virtual void fromXML( const XMLNode::Pointer& node )  { }
    virtual bool toXML( XMLNode::Pointer& node ) const { return false; }
    virtual const DLString & getType( ) const { return DLString::emptyString; }
};

SkillCommand::Pointer Skill::getCommand( ) const
{
    return DummySkillCommand::Pointer( NEW );
}

#include "subprofession.h"
#include "craft_utils.h"
#include "craftattribute.h"

#include "grammar_entities_impl.h"
#include "pcharacter.h"
#include "alignment.h"
#include "infonet.h"
#include "wiznet.h"
#include "room.h"
#include "race.h"
#include "merc.h"
#include "dl_strings.h"
#include "def.h"


/*-------------------------------------------------------------------
 * CraftProfessionHelp 
 *------------------------------------------------------------------*/
const DLString CraftProfessionHelp::TYPE = "CraftProfessionHelp";

void CraftProfessionHelp::setProfession( CraftProfession::Pointer prof )
{
    StringSet kwd;

    this->prof = prof;
    
    if (!keyword.empty( ))
	kwd.fromString( keyword );

    kwd.insert( prof->getName( ) );
    kwd.insert( prof->getRusName( ).ruscase( '1' ) );
    kwd.insert( prof->getMltName( ).ruscase( '1' ) );
    fullKeyword = kwd.toString( ).toUpper( );

    helpManager->registrate( Pointer( this ) );
}

void CraftProfessionHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    fullKeyword = "";
}

void CraftProfessionHelp::getRawText( Character *ch, ostringstream &in ) const
{
    in << "Дополнительная профессия {C" << prof->getRusName( ).ruscase( '1' ) << "{x или {C"
       << prof->getName( ) << "{x" << endl << endl;
        
    in << *this << endl;
}

/*-------------------------------------------------------------------
 * CraftProfession
 *------------------------------------------------------------------*/
CraftProfession::CraftProfession( )
{
}

CraftProfession::~CraftProfession( )
{
}

void CraftProfession::loaded( )
{
    craftProfessionManager->load( Pointer( this ) );

    if (help)
	help->setProfession( Pointer( this ) );
}

void CraftProfession::unloaded( )
{
    if (help)
	help->unsetProfession( );

    craftProfessionManager->load( Pointer( this ) );
}

DLString CraftProfession::getNameFor( Character *ch, const Grammar::Case &c ) const
{
    if (ch && ch->getConfig( )->rucommands)
	return getRusName( ).ruscase( c );
    else
	return getName( );
}

int CraftProfession::getLevel( PCharacter *ch ) const
{
    return craft_attr(ch)->proficiencyLevel(getName());
}

void CraftProfession::setLevel( PCharacter *ch, int level ) const
{
    craft_attr(ch)->setProficiencyLevel(getName(), level);
}

int CraftProfession::getExpToLevel( PCharacter *ch, int level ) const
{ 
    if (level < 0)
        level = getLevel(ch);
    return max(0, getExpPerLevel(ch,  level + 1) - getTotalExp(ch));
}

int CraftProfession::getExpThisLevel( PCharacter *ch ) const
{
    int level = getLevel(ch);
    return getExpPerLevel(ch, level + 1) - getExpPerLevel(ch, level);
}

int CraftProfession::getExpPerLevel( PCharacter *ch, int level ) const
{
    if (level < 0)
        level = getLevel(ch);

    // Summ[x=1..lvl] (base + maxLevel * x)
    // TNL: base + maxLevel * maxLevel * (level + 1)
   return baseExp * level + maxLevel * maxLevel * level * (level + 1) / 2;
}

int CraftProfession::getTotalExp( PCharacter *ch ) const
{
    return craft_attr(ch)->exp(getName());
}

int CraftProfession::gainExp( PCharacter *ch, int xp ) const
{
    XMLAttributeCraft::Pointer attr = craft_attr(ch);
    int level = attr->proficiencyLevel(getName());
    int total_xp = attr->gainExp(getName(), xp);

    ch->pecho("Ты получаешь %1$d очк%1$Iо|а|ов опыта в профессии %2$N2.", xp, getRusName().c_str());

    if (level >= maxLevel)
    	return total_xp;
    
    while (getExpToLevel(ch) <= 0) {
        level++;
        attr->setProficiencyLevel(getName(), level);
        ch->pecho("{CТы достигаешь {Y%1$dго{C уровня мастерства в профессии {Y%2$N2{C!",
                   level, getRusName().c_str());

        infonet("{CРадостный голос из $o2: {W$C1 дости$Gгло|г|гла новой ступени профессионального мастерства.{x", 
                 ch, 0);

        wiznet(WIZ_LEVELS, 0, 0, 
	          "%1$^C1 дости%1$Gгло|г|гла %2$d уровня в профессии %3$N2!", 
                  ch, level, getRusName().c_str());
    }
}

/*-------------------------------------------------------------------
 * CraftProfessionManager
 *------------------------------------------------------------------*/
CraftProfessionManager * craftProfessionManager = 0;

CraftProfessionManager::CraftProfessionManager( )
{
    checkDuplicate( craftProfessionManager );
    craftProfessionManager = this;
}

CraftProfessionManager::~CraftProfessionManager( )
{
    craftProfessionManager = 0;
}

void CraftProfessionManager::load( CraftProfession::Pointer prof )
{
    profs[prof->getName()] = prof;
}

void CraftProfessionManager::unload( CraftProfession::Pointer prof )
{
    Professions::iterator p = profs.find(prof->getName());
    profs.erase(p);
}

CraftProfession::Pointer CraftProfessionManager::get( const DLString &name ) const
{
    Professions::const_iterator p = profs.find(name);
    if (p == profs.end())
        return CraftProfession::Pointer();
    else
        return p->second;
}

CraftProfession::Pointer CraftProfessionManager::lookup( const DLString &arg ) const
{
    Professions::const_iterator p;
    for (p = profs.begin(); p != profs.end(); p++)
        if (is_name(arg.c_str(), p->first.c_str()))
            return p->second;
    return CraftProfession::Pointer();
}

list<CraftProfession::Pointer> CraftProfessionManager::getProfessions() const
{
    list<CraftProfession::Pointer> result;
    Professions::const_iterator p;
    for (p = profs.begin(); p != profs.end(); p++)
        result.push_back(p->second);
    return result;
}

void CraftProfessionManager::initialization( ) 
{
}

void CraftProfessionManager::destruction( ) 
{
}


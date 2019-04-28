#include "subprofession.h"
#include "craft_utils.h"
#include "craftattribute.h"
#include "logstream.h"
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

static const DLString LABEL_CRAFT = "craft";

/*-------------------------------------------------------------------
 * CraftProfessionHelp 
 *------------------------------------------------------------------*/
const DLString CraftProfessionHelp::TYPE = "CraftProfessionHelp";

DLString CraftProfessionHelp::getTitle(const DLString &label) const
{
    if (prof)
        return prof->getRusName().ruscase('1') + ", " + prof->getName();
    return HelpArticle::getTitle(label);
}
    
void CraftProfessionHelp::setProfession( CraftProfession::Pointer prof )
{
    this->prof = prof;
    
    if (!keyword.empty( ))
        keywords.fromString( keyword.toLower() );

    keywords.insert( prof->getName( ) );
    keywords.insert( prof->getRusName( ).ruscase( '1' ) );
    keywords.insert( prof->getMltName( ).ruscase( '1' ) );
    fullKeyword = keywords.toString( ).toUpper( );
    addLabel(LABEL_CRAFT);

    helpManager->registrate( Pointer( this ) );
}

void CraftProfessionHelp::unsetProfession( )
{
    helpManager->unregistrate( Pointer( this ) );
    prof.clear( );
    keywords.clear();
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

int ExperienceCalculator::expPerLevel(int level_) const
{
    throw Exception("not implemented");
}
int ExperienceCalculator::expThisLevel() const
{
    throw Exception("not implemented");
}
int ExperienceCalculator::expToLevel(int level_) const
{
    throw Exception("not implemented");
}
int ExperienceCalculator::totalExp() const
{
    throw Exception("not implemented");
}

struct DefaultExperienceCalculator : public ExperienceCalculator {
    typedef ::Pointer<DefaultExperienceCalculator> Pointer;

    DefaultExperienceCalculator(PCharacter *ch, const CraftProfession *profession) 
    {
        XMLAttributeCraft::Pointer attr = craft_attr(ch);
        level = attr->proficiencyLevel(profession->getName());
        totalExp_ = attr->exp(profession->getName());
        baseExp = profession->getBaseExp();
        maxLevel = profession->getMaxLevel();
    }
    
    /**
     * Return total amount of experience you needed to gain from level 1 to reach this particular level.
     * If level is unspecified, current one is assumed.
     */
    virtual int expPerLevel(int level_ = -1) const
    {
        if (level_ < 0)
            level_ = level;
            
        // Summ[x=1..lvl] (base + maxLevel * x)
        // TNL: base + maxLevel * maxLevel * (level + 1)
        return baseExp * level_ + maxLevel * maxLevel * level_ * (level_ + 1) / 2;
    }

    /**
     * Return total amount of experience you need to gain at your current proficiency
     * level, in order to progress to the next one.
     */
    virtual int expThisLevel() const
    {
        return expPerLevel(level + 1) - expPerLevel(level);
    }

    /**
     * Return how much exp is left to gain to reach next level after this particular one.
     * If level is unspecified, current one is assumed.
     */
    virtual int expToLevel(int level_ = -1) const
    {
        if (level_ < 0)
            level_ = level;

        int totalExpForNextLevel = expPerLevel(level_ + 1);
        return max(0, totalExpForNextLevel - totalExp_);
    }
    
    /**
     * Return total amount of experience gained so far.
     */
    virtual int totalExp() const
    {
        return totalExp_;
    }

protected:
    int level;
    int totalExp_;
    int baseExp;
    int maxLevel;
};

int CraftProfession::getLevel( PCharacter *ch ) const
{
    return craft_attr(ch)->proficiencyLevel(getName());
}

void CraftProfession::setLevel( PCharacter *ch, int level ) const
{
    XMLAttributeCraft::Pointer attr = craft_attr(ch);
    // Set the new level.
    attr->setProficiencyLevel(getName(), level);

    // Update exp so that remaining tnl is never greater than total exp to next level.
    ExperienceCalculator::Pointer calc = getCalculator(ch);
    int totalForNextLevel = calc->expThisLevel();
    int remainingForNextLevel = calc->expToLevel();
    if (remainingForNextLevel > totalForNextLevel)
        attr->gainExp(getName(), remainingForNextLevel - totalForNextLevel);
}

ExperienceCalculator::Pointer CraftProfession::getCalculator(PCharacter *ch) const
{
    return DefaultExperienceCalculator::Pointer(NEW, ch, this);
}

void CraftProfession::gainExp( PCharacter *ch, int xp ) const
{
    XMLAttributeCraft::Pointer attr = craft_attr(ch);
    int level = attr->proficiencyLevel(getName());

    attr->gainExp(getName(), xp);

    ch->pecho("Ты получаешь %1$d очк%1$Iо|а|ов опыта в профессии %2$N2.", xp, getRusName().c_str());

    if (level >= maxLevel) {
        ch->save();
        return;
    }
   
    ExperienceCalculator::Pointer calc = getCalculator(ch);
 
    while (calc->expToLevel(level) <= 0) {
        level++;
        attr->setProficiencyLevel(getName(), level);
        ch->pecho("{CТы достигаешь {Y%1$dго{C уровня мастерства в профессии {Y%2$N2{C!",
                   level, getRusName().c_str());

        infonet("{CРадостный голос из $o2: {W$C1 дости$Gгло|г|гла новой ступени мастерства.{x", 
                 ch, 0);

        wiznet(WIZ_LEVELS, 0, 0, 
                  "%1$^C1 дости%1$Gгло|г|гла %2$d уровня в профессии %3$N2!", 
                  ch, level, getRusName().c_str());
    }

    ch->updateSkills();
    ch->save();
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


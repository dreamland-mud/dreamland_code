/* $Id: genericskill.cpp,v 1.1.2.23.6.15 2010-09-05 13:57:10 rufina Exp $
 *
 * ruffina, 2004
 */
#include <set>
#include "genericskill.h"

#include "logstream.h"
#include "stringlist.h"
#include "grammar_entities_impl.h"
#include "skillmanager.h"
#include "skillreference.h"
#include "skillgroup.h"
#include "skill_utils.h"

#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "hometown.h"
#include "profflags.h"

#include "act.h"
#include "clan.h"
#include "merc.h"
#include "def.h"

PROF(none);
PROF(vampire);
GROUP(none);

const DLString GenericSkill::CATEGORY = "Классовые умения";

GenericSkill::GenericSkill( ) 
                : group(skillGroupManager),
                  raceBonuses(raceManager),
                  classes( false ),
                  hidden( false )

{
}

GenericSkill::~GenericSkill( )
{
}

void GenericSkill::loaded( )
{
    BasicSkill::loaded();

    // Assign additional per-class labels to help articles.
    if (help) {
        for (auto &pair: classes)
            help->labels.addTransient(pair.first + "-skills");
    }
}

GlobalBitvector & GenericSkill::getGroups()
{
    return group;
}

/** Return true if this skill belongs to at least one visible class/profession. */
bool GenericSkill::isProfessional() const
{
    if (classes.empty())
        return false;

    for (auto &pair: classes) {
        Profession *prof = professionManager->findExisting(pair.first);
        if (prof && !prof->getFlags().isSet(PROF_NEWLOCK))
            return true;
    }

    return false;
}

bool GenericSkill::checkAlignEthos(Character *ch) const
{
    if (ch && ch->alignment != ALIGN_NONE)
        if (align.getValue() > 0 && !align.isSetBitNumber(ALIGNMENT(ch)))
            return false;

    if (ch && ch->ethos != ETHOS_NULL)
        if (ethos.getValue() > 0 && !ethos.isSetBitNumber(ch->ethos))
            return false;

    return true;
}

/*
 * виден ли скилл этому чару в принципе, независимо от уровня.
 */
bool GenericSkill::visible( CharacterMemoryInterface *mem ) const
{
    const SkillClassInfo *ci;
    Character *ch = 0;
    
    if (hidden.getValue( ))
        return false;

    if (mem->getMobile()) {
        switch (mob.visible( mem->getMobile(), this )) {
        case MPROF_ANY:
            return true;
        case MPROF_NONE:
            return false;
        case MPROF_REQUIRED:
            break;
        }

        ch = mem->getMobile();
    } else if (mem->getPlayer()) {
        ch = mem->getPlayer();
    } 

    if (!checkAlignEthos(ch))
        return false;

    if (hasRaceBonus(mem))
        return true;
    
    ci = getClassInfo( mem );
    if (ci && ci->visible( ))
        return true;

    if (temporary_skill_active(this, mem))
        return true;
 
    return false;
}

/*
 * доступен ли на этом уровне
 */
bool GenericSkill::available( Character *ch ) const
{
    return ch->getRealLevel( ) >= getLevel( ch );
}

/*
 * может ли чар _сейчас_ использовать этот (уже доступный) скилл
 */
bool GenericSkill::usable( Character *ch, bool message = false ) const
{
    if (!available( ch ))
        return false;
    
    if (ch->is_npc( ))
        return true;

    // Vampires have a restriction that spells are only available in vamp form.
    if (ch->getProfession( ) != prof_vampire)
        return true;

    if (!spell)
        return true;

    if (IS_VAMPIRE(ch))
        return true;

    // Race bonuses are always accessible.
    if (hasRaceBonus(ch))
        return true;

    // Dreamt or bonus skills area always accessible.
    if (temporary_skill_active(this, ch))
        return true;

    if (message)
        ch->pecho("Для этого необходимо превратиться в вампира!");

    return false;
}

bool GenericSkill::availableForAll( ) const
{
    for (int i = 0; i < professionManager->size( ); i++) {
        Profession *prof = professionManager->find( i );

        if (prof->isValid( ) 
                && prof->isPlayed( )
                && !classes.isAvailable( prof->getName( ) ))
            return false;
    }
        
    return true; 
}

/*
 * с какого уровня скилл станет доступен этому чару
 * Для мобов: скилы, соответствующие их off_flags, доступны с 1 уровня
 * (например: OFF_DIRT, OFF_KICK)
 */
int GenericSkill::getLevel( Character *ch ) const
{
    const SkillClassInfo *ci;

    if (!visible( ch ))
        return 999;
    
    if (ch->is_npc( )) {
        if (mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
            return 1;
    }

    // Skills acquired from worn items become available immediately.
    if (temporary_skill_active(this, ch))
        return ch->getRealLevel();
 
    // Race bonuses are available immediately.
    if (hasRaceBonus(ch))
        return 1;

    // Return class level or non-zero race bonus level, whatever is lower,
    // e.g. urukhai get spears from level 1.
    ci = getClassInfo( ch );
    if (ci && ci->visible()) {
        int classLevel = ci->getLevel();
        int raceLevel = hasRaceBonus(ch) ? 1 : 0;

        if (raceLevel == 0)
            return classLevel;

        return min(classLevel, raceLevel);
    }

    // Can't be here.
    return ch->getRealLevel();
}

/*
 * Для чаров возвращает процент разученности скила.
 * Для мобов возвращает dice * level + bonus
 */
int GenericSkill::getLearned( Character *ch ) const
{
    if (!usable( ch ))
        return 0;

    if (ch->is_npc( )) 
        return mob.getLearned( ch->getNPC( ), this );
    
    PCharacter *pch = ch->getPC( );
    int percent = pch->getSkillData( getIndex( ) ).learned;

    if (temporary_skill_active(this, ch))
        return percent;
   
    if (hasRaceBonus(pch))
        percent = 100;
    
    return URANGE( 1, percent, 100 );
}

int GenericSkill::getMaximum( Character *ch ) const
{
    const SkillClassInfo *ci;
    if (( ci = getClassInfo( ch ) ))
        return ci->getMaximum( );

    if (temporary_skill_active(this, ch))
        return ch->getProfession( )->getSkillAdept( );

    return BasicSkill::getMaximum( ch );
}

MobSkillData *GenericSkill::getMobSkillData()
{
    return &mob;
}


/*
 * skill rating for player's class (how hard is it to learn)
 */
int GenericSkill::getRating( PCharacter *ch ) const
{
    const SkillClassInfo *ci;

    ci = getClassInfo( ch );
    
    if (ci)
        return ci->getRating( );
        
    return 1;
}

/*
 * может ли чар практиковать этот скилл
 */
bool GenericSkill::canPractice( PCharacter *ch, std::ostream & buf ) const
{
    if (!available( ch ))
        return false;
    
    if (ch->getSkillData(getIndex()).isTemporary()) {
        buf << "Ты уже знаешь '" << getNameFor(ch) << "' так хорошо, как только можешь." << endl;
        return false;
    }
    
    return true;
}

bool GenericSkill::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose ) 
{
    if (!mob) {
        if (verbose)
            ch->pecho( "Тебе не с кем практиковаться здесь." );
        return false;
    }
    
    if (mob->pIndexData->practicer.isSetAny(getGroups()))
        return true;

    if (verbose)
        ch->pecho( "%1$^C1 не может научить тебя искусству '%2$s'.\n"
               "Учителя можно найти, прочитав справку по этому умению.",
               mob, getNameFor( ch ).c_str( ) );
    return false;
}

/*
 * Печатает разную инфу: группу, затраты на выполнение, раскачку, где учить.
 * Используется в showskill и в справке по умению.
 */
void GenericSkill::show( PCharacter *ch, std::ostream & buf ) const
{
    const char *pad = SKILL_INFO_PAD;

    auto fillRacesClassesInfo = [&]() {
        if (!classes.empty())
            buf << pad << "Доступно классам: {D" << this->skillClassesList() << "{x" << endl;
        if (!raceBonuses.empty())
            buf << pad << "Бонус для рас: {D" << this->skillRacesList() << "{x" << endl;
    };

    buf << print_what(this) << " "
        << print_names_for(this, ch)
        << print_group_for(this, ch)
        << ".{x" << endl
        << printWaitAndMana(ch);

    if (!visible( ch )) {
        if (!classes.empty() && ch->getProfession() != prof_none)
            buf << pad << "Недоступно для твоего класса или характера." << endl;
        fillRacesClassesInfo();
        return;
    }

    const PCSkillData &data = ch->getSkillData(getIndex());
    int percent = data.learned;
    if (temporary_skill_active(this, ch)) {
        if (data.origin == SKILL_DREAM)
            buf << pad << "Приснилось тебе";
        else
            buf << pad << "Досталось тебе";
        buf << " разученное до {C" << percent << "%{x"
            << skill_effective_bonus(this, ch) << "." << endl;
        fillRacesClassesInfo();
        return;
    }

    if (!available(ch)) {
        buf << pad << "Станет доступно тебе на уровне {C" << getLevel(ch) << "{x." << endl;
    } else {
        buf << pad << "Доступно тебе с уровня {C" << getLevel(ch) << "{x, ";
        if (percent < 2) 
            buf << "пока не изучено";
        else 
            buf << "изучено на {" << skill_learned_colour(this, ch) << percent << "%{x";
        
        buf << skill_effective_bonus(this, ch) << "." << endl;
    }

    buf << printPracticers(ch);
    buf << printLevelBonus(ch);
    fillRacesClassesInfo();
}

/*
 * Возвращает структуру SkillClassInfo для класса этого чара.
 * Для мобов ищет тот класс, в котором скил доступен на самом низком левеле.
 * Мобы могут быть "многоклассовыми", в соотв-и со своими act-flags.
 */
const SkillClassInfo *
GenericSkill::getClassInfo( CharacterMemoryInterface *ch ) const
{
    vector<int> proffi = ch->getProfession( )->toVector( ch ).toArray( );
    int minLevel = LEVEL_IMMORTAL;
    const SkillClassInfo *bestClass = 0;
    
    for (unsigned int i = 0; i < proffi.size( ); i++) {
        Classes::const_iterator iter = classes.find( 
                    professionManager->find( proffi[i] )->getName( ) );

        if (iter != classes.end( ) && iter->second.getLevel( ) < minLevel) {
            minLevel = iter->second.getLevel( );
            bestClass = &iter->second;
        }
    }
    
    return bestClass;
}


bool GenericSkill::hasRaceBonus( CharacterMemoryInterface *ch ) const
{
    return raceBonuses.isSet(ch->getRace());
}

/*--------------------------------------------------------------------------
 * SkillClassInfo
 *--------------------------------------------------------------------------*/
SkillClassInfo::SkillClassInfo( )
                 : rating(1), maximum( 100 )
{
}

bool SkillClassInfo::visible( ) const
{
    return getLevel( ) < LEVEL_IMMORTAL 
            && getRating( ) > 0;
}

/*--------------------------------------------------------------------------
 * OLC helpers
 *--------------------------------------------------------------------------*/

bool GenericSkill::accessFromString(const DLString &newValue, ostringstream &errBuf)
{
    StringList values;
    values.split(newValue, ";");
    DLString classValues = values.front().stripWhiteSpace();
    DLString raceBonusValues = values.size() > 1 ? values.back().stripWhiteSpace() : DLString::emptyString;

    map<DLString, int> newClasses = parseAccessTokens(classValues, professionManager, errBuf);
    if (!errBuf.str().empty())
        return false;

    if (newClasses.empty()) {
        // Valid empty input, flush all class info from this skill.
        if (!classes.empty())
            errBuf << "Все классовые ограничения очищены." << endl;
        classes.clear();
    } 

    if (raceBonusValues.empty()) {
        // Valid empty race bonus input.
        if (!raceBonuses.empty())
            errBuf << "Все расовые бонусы очищены." << endl;
        raceBonuses.clear();
    } else {
        raceBonuses.fromString(raceBonusValues);
    }

    // Adjust existing class levels or create new elements.
    for (auto &newPair: newClasses) {
        auto c = classes.find(newPair.first);
        if (c == classes.end()) {
            classes[newPair.first].level = newPair.second;
        } else {
            c->second.level = newPair.second;
        }
    }

    // Wipe class info no longer present in the input.
    for (auto c = classes.begin(), last = classes.end(); c != last; ) {
        if (newClasses.count(c->first) == 0)
            c = classes.erase(c);
        else
            c++;
    }

    errBuf << "Новые классовые ограничения и расовые бонусы: " << accessToString() << endl;
    return true;
}

DLString GenericSkill::accessToString() const
{
    StringList classBuf;

    for (auto &c: classes) {
        classBuf.push_back(c.first + " " + DLString(c.second.getLevel()));
    }

    return classBuf.join(", ") +  ";" + raceBonuses.toString(' ');
}

DLString GenericSkill::skillClassesList() const
{
    StringList result;

    for (auto &c: classes) {
        result.push_back(professionManager->find(c.first)->getRusName().ruscase('1'));
    }

    return result.join("{x, {D");
}

DLString GenericSkill::skillRacesList() const
{
    StringList result;

    for (auto &r: raceBonuses.toArray()) {
        result.push_back(raceManager->find(r)->getMltName().ruscase('1'));
    }

    return result.join("{x, {D");
}
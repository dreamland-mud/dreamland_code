/* $Id: genericskill.cpp,v 1.1.2.23.6.15 2010-09-05 13:57:10 rufina Exp $
 *
 * ruffina, 2004
 */
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
#include "mercdb.h"
#include "act.h"
#include "clan.h"
#include "merc.h"
#include "def.h"

PROF(none);
PROF(vampire);
HOMETOWN(frigate);
GROUP(none);

const DLString GenericSkill::CATEGORY = "Классовые умения";

GenericSkill::GenericSkill( ) 
                : group(skillGroupManager),
                  raceAffect( 0, &affect_flags ),
                  raceBonuses( false ),
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

/*
 * виден ли скилл этому чару в принципе, независимо от уровня.
 */
bool GenericSkill::visible( CharacterMemoryInterface *mem ) const
{
    const SkillRaceBonus *rb; 
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

    if (ch && align.getValue() > 0 && !align.isSetBitNumber(ALIGNMENT(ch)))
        return false;

    if (ch && ethos.getValue() > 0 && !ethos.isSetBitNumber(ch->ethos))
        return false;

    rb = getRaceBonus( mem );
    if (rb && rb->visible( ))
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
    const SkillRaceBonus *rb; 

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
    rb = getRaceBonus( ch );
    if (rb && !rb->isProfessional( ))
        return true;

    // Dreamt or bonus skills area always accessible.
    if (temporary_skill_active(this, ch))
        return true;

    if (message)
        ch->send_to("Для этого необходимо превратиться в вампира!\n\r");

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
    const SkillRaceBonus *rb; 
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
 
    rb = getRaceBonus( ch );
    // Race bonuses that are independent on profession are available immediately,
    // e.g. rockseers get wands from level 1.
    if (rb && !rb->isProfessional( ))
        return rb->getLevel( );

    // Return class level or non-zero race bonus level, whatever is lower,
    // e.g. urukhai get spears from level 1.
    ci = getClassInfo( ch );
    if (ci && ci->visible()) {
        int classLevel = ci->getLevel();
        int raceLevel = rb ? rb->getLevel() : 0;

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

    if (isRaceAffect( pch ))
        return percent;
    
    if (temporary_skill_active(this, ch))
        return percent;
   
    const SkillRaceBonus *rb = getRaceBonus( pch );
    if (rb) 
        percent = std::max( percent, rb->getBonus( ) );
    
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
            ch->println( "Тебе не с кем практиковаться здесь." );
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

    buf << print_what(this) << " "
        << print_names_for(this, ch)
        << print_group_for(this, ch)
        << ".{x" << endl
        << printWaitAndMana(ch);

    if (!visible( ch )) {
        if (!classes.empty() && ch->getProfession() != prof_none)
            buf << pad << "Недоступно для твоего класса." << endl;
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

    const DLString what = skill_what(this).ruscase('1');
    list<MOB_INDEX_DATA *> practicers;

    for (auto g: const_cast<GenericSkill *>(this)->getGroups().toArray()) {
        SkillGroup *group = skillGroupManager->find(g);
        if (group->getPracticer() <= 0)
            continue;

        MOB_INDEX_DATA *pMob = get_mob_index(group->getPracticer());
        if (pMob)
            practicers.push_back(pMob);
    }

    if (practicers.empty()) {
        // '...в твоей гильдии' - с гипер-ссылкой на справку.
        buf << pad << "Это " << what << " можно выучить в твоей {g{hh44гильдии{x." << endl;
    } else {
        // 'Это заклинание можно выучить у Маршала Дианы (зона Новый Офкол)' - с гипер-ссылкой на зону
        buf << pad << "Это " << what << " можно выучить у ";
        
        MOB_INDEX_DATA *pMob = practicers.front();
        buf << "{g" << russian_case( pMob->short_descr, '2' ) << "{x "
                << "(зона {g{hh" << pMob->area->name << "{x)";

        // For multi-groups show two teachers only.
        if (practicers.size() > 1) {
            pMob = practicers.back();
            buf << " или у {g" << russian_case( pMob->short_descr, '2' ) << "{x "
                    << "(зона {g{hh" << pMob->area->name << "{x)";
        }
                    
        buf << "." << endl;
    }
    
    if (ch->getHometown() == home_frigate)
        buf << pad << "Пока ты на корабле, обращайся к {gКацману{x (Лазарет) или к {gЭткину{x (Арсенал)." << endl;
    else if (ch->getModifyLevel() < 20)
        buf << pad << "Ты все еще можешь учиться у {gадепта{x ({g{hhMUD Школа{x)." << endl;

    buf << printLevelBonus(ch);
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


/*
 * возвращает инфо о расовом бонусе для чара (if any)
 */
const SkillRaceBonus *
GenericSkill::getRaceBonus( CharacterMemoryInterface *ch ) const
{
    RaceBonuses::const_iterator i = raceBonuses.find( ch->getRace( )->getName( ) );

    return (i == raceBonuses.end( ) ? NULL : &(i->second));
}

/*
 * соответствует ли этот скил какому-либо расовому аффекту для чара?
 * (пример: sneak - AFF_SNEAK)
 */
bool GenericSkill::isRaceAffect( CharacterMemoryInterface *ch ) const
{
    return ch->getRace( )->getAff( ).isSet( raceAffect.getValue( ) );
}

/*--------------------------------------------------------------------------
 * SkillRaceBonus
 *--------------------------------------------------------------------------*/
bool SkillRaceBonus::visible( ) const
{
    return !isProfessional( ) 
           && getLevel( ) < LEVEL_IMMORTAL;
}

/*--------------------------------------------------------------------------
 * SkillClassInfo
 *--------------------------------------------------------------------------*/
SkillClassInfo::SkillClassInfo( )
                 : rating(1), maximum( 100 ), always( false ), clanAntiBonuses( false )
{
}

/*
 * возвращает инфо о клановых запретах на использования скила 
 * для данного класса
 */
const SkillClanAntiBonus *
SkillClassInfo::getClanAntiBonus( CharacterMemoryInterface *ch ) const
{
    ClanAntiBonuses::const_iterator i;

    i = clanAntiBonuses.find( ch->getClan( )->getName( ) );

    return (i == clanAntiBonuses.end( ) ? NULL : &(i->second));
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
    // Assume only class restrictions are passed here for now.

    map<DLString, int> newClasses = parseAccessTokens(newValue, professionManager, errBuf);

    if (newClasses.empty() && errBuf.str().empty()) {
        // Valid empty input, flush all class info from this skill.
        classes.clear();
        errBuf << "Все классовые ограничения очищены." << endl;
        return true;
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

    errBuf << "Новые классовые ограничения: " << accessToString() << endl;
    return true;
}

DLString GenericSkill::accessToString() const
{
    StringList result;

    for (auto &c: classes) {
        result.push_back(c.first + " " + DLString(c.second.getLevel()));
    }

    return result.join(", ");
}

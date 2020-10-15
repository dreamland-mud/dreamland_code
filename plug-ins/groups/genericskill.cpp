/* $Id: genericskill.cpp,v 1.1.2.23.6.15 2010-09-05 13:57:10 rufina Exp $
 *
 * ruffina, 2004
 */
#include "genericskill.h"
#include "genericskillloader.h"

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
#include "clan.h"
#include "merc.h"
#include "def.h"

PROF(none);
PROF(vampire);
HOMETOWN(frigate);
GROUP(none);

const DLString GenericSkill::CATEGORY = "Профессиональные умения";

GenericSkill::GenericSkill( ) 
                : raceAffect( 0, &affect_flags ),
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

SkillGroupReference & GenericSkill::getGroup( )
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

void GenericSkill::resolve( ) 
{
}

void GenericSkill::unresolve( )
{
}

/*
 * виден ли скилл этому чару в принципе, независимо от уровня.
 */
bool GenericSkill::visible( Character *ch ) const
{
    const SkillRaceBonus *rb; 
    const SkillClassInfo *ci;
    
    if (hidden.getValue( ))
        return false;

    if (ch->is_npc( )) {
        switch (mob.visible( ch->getNPC( ), this )) {
        case MPROF_ANY:
            return true;
        case MPROF_NONE:
            return false;
        case MPROF_REQUIRED:
            break;
        }
    }
    
    rb = getRaceBonus( ch );
    if (rb && rb->visible( ))
        return true;
    
    ci = getClassInfo( ch );
    if (ci && ci->visible( ))
        return true;

    if (temporary_skill_active(this, ch))
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

    rb = getRaceBonus( ch );
    if (rb && !rb->isProfessional( ))
        return true;

    if (ch->getProfession( ) == prof_vampire) {
        if (spell && !IS_VAMPIRE( ch )) {
            if (message)
                ch->send_to("Для этого необходимо превратиться в вампира!\n\r");

            return false;
        }
        else
            return true;
    }
    else
        return true;

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
 * Для чаров возвращает процент разученности скила, с учетом всех скилов-предков.
 * Для мобов возвращает dice * level + bonus
 */
int GenericSkill::getLearned( Character *ch ) const
{
    PCharacter *pch;
    int adept;
    
    if (!usable( ch ))
        return 0;

    if (ch->is_npc( )) 
        return mob.getLearned( ch->getNPC( ), this );
    
    pch = ch->getPC( );

    if (isRaceAffect( pch ))
        return pch->getSkillData( getIndex( ) ).learned;
    
    if (temporary_skill_active(this, ch))
        return pch->getSkillData( getIndex( ) ).learned;
   
    adept = pch->getProfession( )->getSkillAdept( );
            
    return learnedAux( pch, adept );
}

/*
 * вспомогательная процедура для getLearned
 * находит минимально разученный скил среди всех предков
 * (без учета скилов, разученных > 75 или совпадающих с расовыми аффектами)
 */
int GenericSkill::learnedAux( PCharacter *pch, int adept ) const
{
    const SkillRaceBonus *rb;
    int percent, min;
    
    if (!available( pch )) {
        LogStream::sendError( ) << "parent skill " << getName( ) << " is not available  for " << pch->getName( ) << endl;
        return 0;
    }
        
    min = 100;    
    percent = pch->getSkillData( getIndex( ) ).learned;
    
    rb = getRaceBonus( pch );
    
    if (rb) 
        percent = std::max( percent, rb->getBonus( ) );
    
    if (isRaceAffect( pch ))
        return min;
    else
        return URANGE( 1, percent, min );
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
    
    if (mob->pIndexData->practicer.isSet( (int)getGroup( ) ))
        return true;

    if (verbose)
        ch->pecho( "%1$^C1 не может научить тебя искусству '%2$s'.\n"
               "Для большей информации используй команду {y{hc{lRумение %2$s{lEslook %2$s{x.",
               mob, getNameFor( ch ).c_str( ) );
    return false;
}

/*
 * Печатает разную инфу: группу, цену в s.p., дерево предков, список потомков etc
 * Используется в showskill.
 */
void GenericSkill::show( PCharacter *ch, std::ostream & buf ) const
{
    const DLString what = skill_what(this).ruscase('1');
    SkillGroupReference &group = const_cast<GenericSkill *>(this)->getGroup();

    buf << what.upperFirstCharacter()
        << " '{c" << getName( ) << "{x' или"
        << " '{c" << getRussianName( ) << "{x'";
    if (group != group_none)
        buf << ", входит в группу '{hg{c" << group->getNameFor(ch) << "{x'";
    buf << "." << endl;
    
    print_wait_and_mana(this, ch, buf);            
    buf << endl;
    
    if (!visible( ch )) {
        if (!classes.empty())
            buf << "Недоступно для твоей профессии." << endl;
        print_see_also(this, ch, buf);
        return;
    }

    const PCSkillData &data = ch->getSkillData(getIndex());
    int percent = data.learned;
    if (temporary_skill_active(this, ch)) {
        if (data.origin == SKILL_DREAM)
            buf << "Приснилось тебе";
        else
            buf << "Досталось тебе";
        buf << " разученное до {C" << percent << "%{x"
            << skill_effective_bonus(this, ch) << "." << endl;
        print_see_also(this, ch, buf);
        return;
    }

    if (!available(ch)) {
        buf << "Станет доступно тебе на уровне {C" << getLevel(ch) << "{x." << endl;
    } else {
        buf << "Доступно тебе с уровня {C" << getLevel(ch) << "{x, ";
        if (percent < 2) 
            buf << "пока не изучено";
        else 
            buf << "изучено на {" << skill_learned_colour(this, ch) << percent << "%{x";
        
        buf << skill_effective_bonus(this, ch) << "." << endl;
    }
    
    if (group->getPracticer() == 0) {
        // '...в твоей гильдии' - с гипер-ссылкой на справку.
        buf << "Это " << what << " можно выучить в твоей {g{hh44гильдии{x." << endl;
    } else {
        // 'Это заклинание можно выучить у Маршала Дианы (зона Новый Офкол)' - с гипер-ссылкой на зону
        MOB_INDEX_DATA *pMob = get_mob_index(group->getPracticer());
        if (pMob)
            buf << "Это " << what << " можно выучить у "
                << "{g" << russian_case( pMob->short_descr, '2' ) << "{x "
                << "(зона {g{hh" << pMob->area->name << "{x)." << endl;
    }
    
    if (ch->getHometown() == home_frigate)
        buf << "Пока ты на корабле, обращайся к {gКацману{x (Лазарет) или к {gЭткину{x (Арсенал)." << endl;
    else if (ch->getModifyLevel() < 20)
        buf << "Ты все еще можешь учиться у {gадепта{x ({g{hhMUD Школа{x)." << endl;

    print_see_also(this, ch, buf);
}

/*
 * Возвращает структуру SkillClassInfo для класса этого чара.
 * Для мобов ищет тот класс, в котором скил доступен на самом низком левеле.
 * Мобы могут быть "многоклассовыми", в соотв-и со своими act-flags.
 */
const SkillClassInfo *
GenericSkill::getClassInfo( Character *ch ) const
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

SkillClassInfo *
GenericSkill::getClassInfo( PCharacter *ch ) 
{
    Classes::iterator iter = classes.find( ch->getProfession( )->getName( ) );

    if (iter == classes.end( ) || iter->second.getClanAntiBonus( ch ))
        return NULL;
    else 
        return &iter->second;
}

SkillClassInfo * 
GenericSkill::getClassInfo( const DLString &className )
{
    GenericSkill::Classes::iterator c;

    c = classes.find( className );

    if (c == classes.end( ))
        throw Exception( "Skill " + getName( ) + " declared as parent, "
                         "doesnt have entry for " + className + "'" );

    return &c->second;
}

/*
 * возвращает инфо о расовом бонусе для чара (if any)
 */
const SkillRaceBonus *
GenericSkill::getRaceBonus( Character *ch ) const
{
    RaceBonuses::const_iterator i = raceBonuses.find( ch->getRace( )->getName( ) );

    return (i == raceBonuses.end( ) ? NULL : &(i->second));
}

/*
 * соответствует ли этот скил какому-либо расовому аффекту для чара?
 * (пример: sneak - AFF_SNEAK)
 */
bool GenericSkill::isRaceAffect( Character *ch ) const
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
                 : maximum( 100 ), always( false ), clanAntiBonuses( false )
{
}

/*
 * возвращает инфо о клановых запретах на использования скила 
 * для данной профессии
 */
const SkillClanAntiBonus *
SkillClassInfo::getClanAntiBonus( Character *ch ) const
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


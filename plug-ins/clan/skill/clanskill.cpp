/* $Id: clanskill.cpp,v 1.1.6.5.6.13 2009/03/16 20:06:59 rufina Exp $
 *
 * ruffina, 2004
 */
#include "clanskill.h"
#include "clantypes.h"

#include "stringlist.h"
#include "skillmanager.h"
#include "skill_utils.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "npcharacter.h"
#include "dreamland.h"
#include "act.h"
#include "merc.h"

#include "def.h"

ClanSkill::ClanSkill( )
            : group(skillGroupManager)
{
}

void ClanSkill::loaded( )
{
    BasicSkill::loaded();

    // Assign additional per-clan labels to help articles.
    if (help) {
        for (auto &pair: clans)
            help->labels.addTransient(pair.first + "-skills");
    }    
}

GlobalBitvector & ClanSkill::getGroups( ) 
{
    return group;
}

bool ClanSkill::visible( CharacterMemoryInterface * ch ) const
{
    const SkillClanInfo *ci;
    
    if (ch->getMobile( ) && mob.visible( ch->getMobile( ), this ) == MPROF_ANY)
        return true;

    if (temporary_skill_active(this, ch))
        return true;

    if (!( ci = getClanInfo( ch ) ))
        return false;

    if (ci->level.getValue( ) >= LEVEL_IMMORTAL)
        return false;

    if (ch->getPCM() && ci->clanLevel.getValue( ) > ch->getPCM()->getClanLevel( ))
        return false;

    if (ci->maxLevel.getValue( ) < ch->getLevel() && ci->maxLevel.getValue( ) < LEVEL_MORTAL)
        return false;

    return true;
}

bool ClanSkill::available( Character * ch ) const
{
    return ch->getRealLevel( ) >= getLevel( ch );
}

bool ClanSkill::usable( Character * ch, bool message = true ) const 
{
    const SkillClanInfo *ci;
    
    if (!available( ch ))
        return false;
    
    if (dreamland->hasOption( DL_BUILDPLOT ))
        return true;

    if (temporary_skill_active(this, ch))
        return true;

    ci = getClanInfo( ch );
    if (ci && !ci->needItem.getValue( ))
        return true;

    if (ch->getClan( )->getData( )
        && ch->getClan( )->getData( )->hasItem( ))
        return true;

    if (message)
        ch->pecho( "Клан не может сейчас придать тебе сил." );

    return false;
}

int ClanSkill::getLevel( Character *ch ) const
{
    if (!visible( ch ))
        return 999;
    
    if (ch->is_npc( ) && mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
        return 1;

    if (temporary_skill_active(this, ch))
        return ch->getRealLevel();
    
    return getClanInfo( ch )->level.getValue( );
}

int ClanSkill::getLearned( Character *ch ) const
{
    const SkillClanInfo *ci;

    if (!usable( ch, false ))
        return 0;

    if (ch->is_npc( )) 
        return mob.getLearned( ch->getNPC( ), this );
    
    if (( ci = getClanInfo( ch ) ) && !ci->needPractice)
        return getMaximum( ch );

    return ch->getPC( )->getSkillData( getIndex( ) ).learned;
}

int ClanSkill::getMaximum( Character *ch ) const
{
    const SkillClanInfo *ci;

    if (( ci = getClanInfo( ch ) ))
        return ci->maximum;

    return BasicSkill::getMaximum( ch );
}

MobSkillData *ClanSkill::getMobSkillData()
{
    return &mob;
}

bool ClanSkill::canPractice( PCharacter * ch, std::ostream & ) const
{
    const SkillClanInfo *ci;
    
    if (!( ci = getClanInfo( ch ) ))
        return false;

    if (ci->level >= LEVEL_IMMORTAL)
        return false;

    if (!ch->is_npc( ) && ci->clanLevel > ch->getPC( )->getClanLevel( ))
        return false;

    if (ci->maxLevel < ch->getRealLevel( ) && ci->maxLevel < LEVEL_MORTAL)
        return false;

    if (ci->level > ch->getRealLevel())
        return false;

    return true;
}

bool ClanSkill::canTeach( NPCharacter *mob, PCharacter * ch, bool verbose ) 
{
    if (mob && ch->getClan( ) == mob->getClan( ))
        return true;
   
    if (verbose) { 
        if (mob)
            ch->pecho( "%^C1 не служит твоему клану.", mob );
        else
            ch->pecho( "Клановые умения практикуют у служителей клана, "
                         "например, у лекаря или охранника." );
    }

    return false;
}

void ClanSkill::show( PCharacter *ch, std::ostream & buf ) const
{
    StringList clanNames;
    Clans::const_iterator i;
    PCSkillData &data = ch->getSkillData( getIndex( ) );    
    const char *pad = SKILL_INFO_PAD;

    buf << print_what(this) << " "
        << print_names_for(this, ch)
        << ", навык ";

    for (i = clans.begin( ); i != clans.end( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i->first );
        
        if (clan->isValid())
            clanNames.push_back(clan->getRussianName( ).ruscase('2'));
    }

    if (clanNames.empty())
        buf << "неизвестного клана";
    else
        buf << clanNames.join(", ");
    
    buf << "{" << SKILL_HEADER_BG << ".{x" << endl; 

    buf << printWaitAndMana(ch);

    if (!visible( ch ))
        return;

    if (temporary_skill_active(this, ch)) {        
        buf << pad << "Досталось тебе разученным на {" 
            << skill_learned_colour(this, ch) << data.learned << "%{x";
    } else {
        buf << pad << "Доступно тебе с уровня {C" << getLevel( ch ) << "{x";
        if (available( ch ))
            buf << ", изучено на {" << skill_learned_colour(this, ch) << data.learned << "%{x";
    }
    
    buf << "." << endl
        << pad << "Практикуется у {gкланового охранника{x." << endl;

    buf << printLevelBonus(ch);
}

const SkillClanInfo * 
ClanSkill::getClanInfo( CharacterMemoryInterface *ch ) const
{
    Clans::const_iterator i;
    
    i = clans.find( ch->getClan( )->getName( ) );

    return (i == clans.end( ) ? NULL : &i->second);
}

SkillClanInfo::SkillClanInfo( )
                 : level( 1 ), maximum( 100 ), rating( 1 ),
                   clanLevel( 0 ), 
                   needItem( true ), needPractice( true ),
                   maxLevel( LEVEL_MORTAL )
{
}


/*--------------------------------------------------------------------------
 * OLC helpers
 *--------------------------------------------------------------------------*/

bool ClanSkill::accessFromString(const DLString &newValue, ostringstream &errBuf)
{
    map<DLString, int> newClans = parseAccessTokens(newValue, clanManager, errBuf);

    if (newClans.empty() && errBuf.str().empty()) {
        // Valid empty input, flush all clan info from this skill.
        clans.clear();
        errBuf << "Все клановые ограничения очищены." << endl;
        return true;
    }

    // Adjust existing clan levels or create new elements.
    for (auto &newPair: newClans) {
        auto c = clans.find(newPair.first);
        if (c == clans.end()) {
            clans[newPair.first].level = newPair.second;
        } else {
            c->second.level = newPair.second;
        }
    }

    // Wipe clan info no longer present in the input.
    for (auto c = clans.begin(), last = clans.end(); c != last; ) {
        if (newClans.count(c->first) == 0)
            c = clans.erase(c);
        else
            c++;
    }

    errBuf << "Новые клановые ограничения: " << accessToString() << endl;
    return true;
}

DLString ClanSkill::accessToString() const
{
    StringList result;

    for (auto &c: clans) {
        result.push_back(c.first + " " + c.second.level.toString());
    }

    return result.join(", ");
}

int ClanSkill::getCategory() const
{
    return SKILL_CAT_CLAN;
}

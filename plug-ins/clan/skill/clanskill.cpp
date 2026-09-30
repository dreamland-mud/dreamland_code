/* $Id: clanskill.cpp,v 1.1.6.5.6.13 2009/03/16 20:06:59 rufina Exp $
 *
 * ruffina, 2004
 */
#include "clanskill.h"
#include "clantypes.h"
#include "clanrecords.h"
#include "clantreasury.h"
#include "profflags.h"

#include "stringlist.h"
#include "logstream.h"
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
#include "l10n.h"

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

    // A reformed clan's catalog skill stays hidden until the leader buys it; learned % is kept.
    if (!ci->catalog.getValue( ).empty( ) && clan_is_reformed( *ch->getClan( ) )
            && !clan_owns( *ch->getClan( ), ci->catalog.getValue( ) ))
        return false;

    // A reformed clan teaches the skill only to the listed class archetypes; learned % is kept.
    if (ci->archetypes.getValue( ) != 0 && clan_is_reformed( *ch->getClan( ) )
            && !ch->getProfession( )->getFlags( ch ).isSet( ci->archetypes.getValue( ) ))
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
        ch->pecho( _("Клан не может сейчас придать тебе сил.") );

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

    int learned = ch->getPC( )->getSkillData( getIndex( ) ).learned;

    // The stored value is kept as is, so a rank-up gives the rest back.
    if (isRankCapped( ch, ci ))
        learned = min( learned, getMaximum( ch ) );

    return learned;
}

int ClanSkill::getMaximum( Character *ch ) const
{
    const SkillClanInfo *ci;

    if (( ci = getClanInfo( ch ) )) {
        if (isRankCapped( ch, ci ))
            return min( ci->maximum.getValue( ),
                        clan_rank_cap( *ch->getClan( ), ch->getPC( )->getClanLevel( ) ) );

        return ci->maximum;
    }

    return BasicSkill::getMaximum( ch );
}

bool ClanSkill::isRankCapped( Character *ch, const SkillClanInfo *ci ) const
{
    return ci && ci->rankCap.getValue( ) && !ch->is_npc( ) && !ch->is_immortal( )
           && clan_is_reformed( *ch->getClan( ) );
}

int ClanSkill::rankLevelBonus( Skill &skill, Character *ch )
{
    ClanSkill *clanSkill = dynamic_cast<ClanSkill *>( &skill );

    if (!clanSkill || ch->is_npc( ) || !clanSkill->getClanInfo( ch ))
        return 0;

    return clan_rank_level_bonus( *ch->getClan( ), ch->getPC( )->getClanLevel( ) );
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
            ch->pecho( _("%^C1 не служит твоему клану."), mob );
        else
            ch->pecho( _("Клановые умения практикуют у служителей клана, "
                         "например, у лекаря или охранника.") );
    }

    return false;
}

void ClanSkill::show( PCharacter *ch, std::ostream & buf ) const
{
    StringList clanNames;
    Clans::const_iterator i;
    PCSkillData &data = ch->getSkillData( getIndex( ) );    
    const char *pad = SKILL_INFO_PAD;

    lang_t lang = viewerLang(ch);

    for (i = clans.begin( ); i != clans.end( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i->first );

        if (!clan->isValid())
            continue;

        // A hidden clan stays unnamed to outsiders; members and immortals see it.
        if (clan->isHidden() && !ch->is_immortal() && ch->getClan()->getName() != clan->getName())
            continue;

        // Russian and Ukrainian put the clan in the genitive; the English
        // ceremonial name is stored undeclined and has no case to render.
        DLString name = clan->getNameFor( lang );
        if (lang != LANG_EN)
            name = name.ruscase('2');

        clanNames.push_back(name);
    }

    buf << print_what(this, ch) << " "
        << print_names_for(this, ch);

    if (clanNames.empty())
        buf << l(ch, ", навык неизвестного клана");
    else
        buf << fmt(ch, _(", навык %1$s"), clanNames.join(", ").c_str());

    buf << "{" << SKILL_HEADER_BG << ".{x" << endl;

    buf << printWaitAndMana(ch);

    if (!visible( ch ))
        return;

    // The colour letter varies with the percentage, so the coloured number is
    // assembled here and passed into the sentence as a plain argument.
    ostringstream learnedBuf;
    learnedBuf << "{" << skill_learned_colour(this, ch) << data.learned << "%{x";
    DLString learned = learnedBuf.str();

    if (temporary_skill_active(this, ch)) {
        buf << pad << fmt(ch, _("Досталось тебе разученным на %1$s"), learned.c_str());
    } else {
        buf << pad << fmt(ch, _("Доступно тебе с уровня {C%1$d{x"), getLevel( ch ));
        if (available( ch )) {
            buf << fmt(ch, _(", изучено на %1$s"), learned.c_str());

            // getLearned() clamps to the rank cap silently: say so, or a
            // 100% skill that fails looks like a bug.
            if (isRankCapped( ch, getClanInfo( ch ) )) {
                int cap = clan_rank_cap( *ch->getClan( ), ch->getClanLevel( ) );
                if (cap < data.learned)
                    buf << fmt(ch, _(", но ранг в клане дает ему работать лишь на {C%1$d%%{x"), cap);
            }
        }
    }

    buf << "." << endl
        << pad << l(ch, "Практикуется у {gкланового охранника{x.") << endl;

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
                   maxLevel( LEVEL_MORTAL ),
                   rankCap( true ),
                   archetypes( )
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

XMLArchetypes::XMLArchetypes( )
        : XMLFlagsNoEmpty( 0, &prof_flags )
{
}

void XMLArchetypes::fromXML( const XMLNode::Pointer& parent )
{
    XMLFlagsNoEmpty::fromXML( parent );

    XMLNode::Pointer node = parent->getFirstNode( );
    if (!node)
        return;

    DLString args = node->getCData( );
    while (!args.empty( )) {
        DLString word = args.getOneArgument( );

        if (prof_flags.index( word ) == NO_FLAG)
            LogStream::sendWarning( )
                << "Unknown archetype '" << word << "' in <archetypes>"
                << node->getCData( ) << "</archetypes>" << endl;
    }
}

DLString ClanSkill::accessToString() const
{
    StringList result;

    for (auto &c: clans) {
        DLString entry = c.first + " " + c.second.level.toString();

        if (c.second.archetypes.getValue( ) != 0)
            entry += " (" + c.second.archetypes.names( ) + ")";

        result.push_back(entry);
    }

    return result.join(", ");
}

int ClanSkill::getCategory() const
{
    return SKILL_CAT_CLAN;
}

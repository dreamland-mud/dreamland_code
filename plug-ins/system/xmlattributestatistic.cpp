/* $Id: xmlattributestatistic.cpp,v 1.1.2.2.6.3 2008/02/24 17:26:57 rufina Exp $
 *
 * ruffina, 2004
 */

#include "xmlattributestatistic.h"
#include "fenia/register-impl.h"
#include "idcontainer.h"
#include "lex.h"
#include "regcontainer.h"
#include "date.h"
#include "xmlattributes.h"
#include "pcharactermanager.h"
#include "pcmemoryinterface.h"

using namespace Scripting;

XMLAttributeStatistic::XMLAttributeStatistic( ) : shy( false )
{
}

XMLAttributeStatistic::~XMLAttributeStatistic( ) 
{
}

void XMLAttributeStatistic::rememberVictory( const DLString &id )
{
    victories[id]++;
}

void XMLAttributeStatistic::rememberPenalty( const DLString &id )
{
    penalties[id]++;
}

int XMLAttributeStatistic::getVictories( const DLString &id ) const
{
    Victories::const_iterator i = victories.find( id );

    if (i == victories.end( ))
        return 0;
    else
        return i->second.getValue( );
}

const XMLAttributeStatistic::Victories &
XMLAttributeStatistic::getVictories( ) const
{
    return victories;
}

void XMLAttributeStatistic::setVictories( const DLString &id, int count )
{
    victories[id] = count;
}

int XMLAttributeStatistic::getWasted( ) const 
{
    return vasted.getValue( );
}

void XMLAttributeStatistic::setWasted( int value ) 
{
    vasted.setValue( value );
}

int XMLAttributeStatistic::getBonusVictoriesCount( ) const 
{   
    Victories::const_iterator i;
    int cnt = 0;

    for (i = victories.begin( ); i != victories.end( ); i++) {
        const DLString &victoryType = i->first;
        int victoryCount = i->second.getValue();

        auto p = penalties.find(victoryType);
        int victoryPenalty = p == penalties.end() ? 0 : p->second.getValue();
        
        cnt += victoryCount - victoryPenalty;
    }

    return cnt;
}

int XMLAttributeStatistic::getAllVictoriesCount( ) const 
{   
    Victories::const_iterator i;
    int cnt = 0;

    for (i = victories.begin( ); i != victories.end( ); i++) {
        int victoryCount = i->second.getValue();
        cnt += victoryCount;
    }

    return cnt;
}

static bool
__cmp_stat_records__( XMLAttributeStatistic::StatRecord &a, XMLAttributeStatistic::StatRecord &b)
{
    return a.second > b.second;
}

XMLAttributeStatistic::Statistic
XMLAttributeStatistic::gatherAll( const DLString &name )
{
    Statistic stat;
    Statistic::iterator s;
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    static time_t cutoff = 1516851059; // 1/25/2018
    
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        Pointer attr;
        PCMemoryInterface *pc;

        pc = i->second;

        if (pc->getLastAccessTime().getTime() < cutoff)
            continue;

        attr = pc->getAttributes( ).findAttr<XMLAttributeStatistic>( name );

        if (attr && !attr->shy) 
            attr->gather( pc, stat );
    }

    for (s = stat.begin( ); s != stat.end( ); s++) {
        StatRecordList &records = s->second;

        records.sort( __cmp_stat_records__ );
    }        

    return stat;
}

void XMLAttributeStatistic::gather( PCMemoryInterface *pc, XMLAttributeStatistic::Statistic &stat ) const
{
    Victories::const_iterator j;

    for (j = victories.begin( ); j != victories.end( ); j++)
        if (j->second.getValue( ) > 0)                
            stat[j->first].push_back( 
                    make_pair( pc->getName( ), j->second.getValue( ) ) );
}

bool XMLAttributeStatistic::handle( const RemortArguments &args ) 
{
    vasted = 0;
    return RemortAttribute::handle( args );
}

Scripting::Register XMLAttributeStatistic::toRegister() const
{
    Register statReg = Register::handler<RegContainer>();
    RegContainer *statContainer = statReg.toHandler().getDynamicPointer<RegContainer>();

    for (auto victory: victories) {
        statContainer->setField(victory.first, victory.second.getValue());
    }
        
    return statReg;
}

Scripting::Register XMLAttributeStatistic::toRankRegister(PCMemoryInterface *pci, const DLString &attrName)
{
    Register rankReg = Register::handler<RegContainer>();
    RegContainer *rankContainer = rankReg.toHandler().getDynamicPointer<RegContainer>();

    auto allStat = gatherAll( attrName );

    for (auto &s: allStat) {
        const DLString &gqName = s.first;
        auto &records = s.second;
        int myplace = 1;
        bool found = false;

        for (auto &r: records) {
            const DLString &playerName = r.first;

            if (playerName == pci->getName()) {
                found = true;
                break;
            }

            myplace++;
        }

        if (found)
            rankContainer->setField(gqName, myplace);
    }

    return rankReg;
}

Scripting::Register XMLAttributeStatistic::toRegister(PCMemoryInterface *player, const DLString &attrName)
{
    Register statReg = Register::handler<IdContainer>();
    IdContainer *statContainer = statReg.toHandler().getDynamicPointer<IdContainer>();
    
    auto statAttr = player->getAttributes().findAttr<XMLAttributeStatistic>(attrName);
    if (!statAttr)
        return statReg;

    statContainer->setField(IdRef("victories"), statAttr->toRegister());
    statContainer->setField(IdRef("rank"), toRankRegister(player, attrName));

    return statReg;

}
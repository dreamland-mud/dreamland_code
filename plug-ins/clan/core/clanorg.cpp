/* $Id$
 *
 * ruffina, 2004
 */
#include "clanorg.h"
#include "grammar_entities_impl.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "arg_utils.h"
#include "logstream.h"
/*----------------------------------------------------------------------------
 * ClanOrder
 *---------------------------------------------------------------------------*/
bool ClanOrder::canInduct( PCMemoryInterface * ) const
{
    return true;
}

const DLString & ClanOrder::getTitle( PCMemoryInterface * ) const
{
    return DLString::emptyString;
}

/*----------------------------------------------------------------------------
 * ClanOrgs
 *---------------------------------------------------------------------------*/
const DLString ClanOrgs::ATTR_NAME = "orden";

/** Do strict and then unstrict org lookup by name or description. */
ClanOrder::Pointer ClanOrgs::findOrder( const DLString &oname ) const
{
    ClanOrder::Pointer null;

    if (oname.size( ) == 0)
        return null;

    const_iterator i = find( oname );
    if (i != end( ))
        return i->second;

    for (auto &co: *this) {
        if (arg_oneof(oname, co.first.c_str()))
            return co.second;
        if (arg_oneof(oname, co.second->shortDescr.c_str()))
            return co.second;
    }

    return null;

}

/** Return the org this player belongs to. */
ClanOrder::Pointer ClanOrgs::findOrder( PCMemoryInterface *pci ) const
{
    return findOrder( getAttr( pci ) );
}

bool ClanOrgs::hasAttr( PCMemoryInterface *pci )
{
    return pci->getAttributes( ).isAvailable( ATTR_NAME );
}

void ClanOrgs::setAttr( PCMemoryInterface *pci, const DLString &value )
{
    pci->getAttributes( ).getAttr<XMLStringAttribute>( ATTR_NAME )->setValue( value );
}

void ClanOrgs::delAttr( PCMemoryInterface *pci )
{
    pci->getAttributes( ).eraseAttribute( ATTR_NAME );
}

const DLString & ClanOrgs::getAttr( PCMemoryInterface *pci )
{
    XMLStringAttribute::Pointer ord;
    
    ord = pci->getAttributes( ).findAttr<XMLStringAttribute>( ATTR_NAME );

    if (ord)
        return ord->getValue( );
    else
        return DLString::emptyString;
}

/*
 * command actions
 */
void ClanOrgs::doList( PCharacter *pch ) const
{
    ostringstream buf;

    for (const_iterator i = begin( ); i != end( ); i++)
        buf << dlprintf("   %-15s (%s)\n\r",
                        i->second->shortDescr.c_str( ),
                        i->first.c_str( ) );

    pch->send_to( buf );
}

void ClanOrgs::doMembers( PCharacter *pch, const DLString &cargs ) const
{
    ostringstream buf;
    ClanOrder::Pointer myOrg, org;
    
    myOrg = findOrder(pch);
    org = findOrder(cargs);
    bool recruiter = pch->getClan()->isRecruiter(pch);

    if (!myOrg && !org) {
        if (recruiter)
            pch->pecho("Укажи правильное название %O2.", &name);
        else
            pch->pecho( "Ты не состоишь в %O6.", &name );
        return;
    }

    if (!myOrg && org && !recruiter) {
        pch->pecho( "Ты не состоишь в %O6.", &name );
        return;
    }

    if (myOrg && org && myOrg->name != org->name && !recruiter) {
        pch->pecho( "Ты не состоишь в эт%1$Gом|ом|ой %1$O6.", &name );
        return;
    }

    ClanOrder::Pointer targetOrg = org ? org : myOrg;

    buf << "\n\r{WИмя         раса        класс         уровень   звание{x\n\r";
    
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    for (PCharacterMemoryList::const_iterator pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;

        if (pcm->getClan( ) != pch->getClan( )
            || pcm->getLevel( ) >= LEVEL_IMMORTAL
            || getAttr( pcm ) != targetOrg->name)
            continue;

        buf << dlprintf("%-10s %-10s %-12s %2d %3d    %-15s\r\n",
                   pcm->getName().c_str(),
                   pcm->getRace()->getName().c_str(),
                   pcm->getProfession( )->getNameFor(pch).c_str(),
                   pcm->getRemorts().size(), 
                   pcm->getLevel(),
                   pcm->getClan()->getTitle(pcm).c_str());
    }

    pch->send_to( buf );
}


void ClanOrgs::doSelfInduct( PCharacter *pch, const DLString &arg ) const
{
    ClanOrder::Pointer ord;

    if (!pch->getClan( )->isRecruiter( pch )) {
        pch->pecho( "Принять себя в %O4 может только рекрутер или лидер.", &name );
        return;
    }
    
    if (!( ord = findOrder( arg ) )) {
        pch->pecho( "%1$^O1 указа%1$Gно|н|на неверно.", &name );
        return;
    }
    
    setAttr( pch, ord->name );
    pch->pecho( "Ты вступаешь в %s.", ord->shortDescr.c_str( ) );
}

void ClanOrgs::doInduct( PCharacter *pch, const DLString &cargs) const
{
    PCMemoryInterface *victim;
    DLString args = cargs;
    DLString argVict = args.getOneArgument();
    DLString argOrder = args;

    if (!pch->getClan( )->isRecruiter( pch )) {
        pch->pecho( "Твоих полномочий недостаточно." );
        return;
    }

    if (!( victim = PCharacterManager::find( argVict ) )) {
        pch->pecho( "Никого нет с таким именем. Укажи имя полностью." );
        return;
    }

    if (victim->getClan( ) != pch->getClan( )) {
        pch->pecho( "Но %s не принадлежит к твоему клану!", victim->getName( ).c_str( ) );
        return;
    }
    
    ClanOrder::Pointer org = findOrder( argOrder );    
    ClanOrder::Pointer myOrg = findOrder(pch);

    pch->pecho("org %s, myOrg %s, argOrder [%s]", org ? org->name.c_str() : "none", 
        myOrg ? myOrg->name.c_str() : "none", argOrder.c_str());

    if (!org && !myOrg) {
        pch->pecho( "%1$^O1 указан%1$Gо||а неверно.", &name );
        return;
    }

    if (!argOrder.empty() && !org) {
        pch->pecho( "%1$^O1 указан%1$Gо||а неверно.", &name );
        return;
    }

    ClanOrder::Pointer targetOrg = org ? org : myOrg;

    if (hasAttr( victim )) {
        if (getAttr( victim ) != targetOrg->name) 
            pch->pecho( "%1$s уже состоит в друг%2$Gом|ом|ой %2$O6.", 
                         victim->getName( ).c_str( ), &name );
        else
            pch->pecho( "%1$s и так состоит в эт%2$Gом|ом|ой %2$O6.", 
                         victim->getName( ).c_str( ), &name );
        
        return;
    }
    
    if (!targetOrg->canInduct( victim )) {
        pch->pecho( "%s не может вступить в %s.", 
                    victim->getName( ).c_str( ), targetOrg->shortDescr.c_str( ) );
        return;
    }
    
    setAttr( victim, targetOrg->name );
    pch->pecho( "Ты принимаешь %s в %s!", 
                 victim->getName( ).c_str( ), targetOrg->shortDescr.c_str( ) );
    
    if (victim->isOnline( ))
        victim->getPlayer( )->pecho( "%s принимает тебя в %s!",
                                     pch->getName( ).c_str( ),
                                     targetOrg->shortDescr.c_str( ) );
    else
        PCharacterManager::saveMemory( victim );
}

void ClanOrgs::doSelfRemove( PCharacter *pch ) const
{
    if (!hasAttr( pch ))
        pch->pecho( "Ты и так нигде не состоишь." );
    else {
        delAttr( pch );
        pch->pecho( "Ты покидаешь сво%1$Gй|й|ю %1$^O4.", &name );
    }
}

void ClanOrgs::doRemove( PCharacter *pch, const DLString &cargs ) const
{
    PCMemoryInterface *victim;
    DLString args = cargs;
    DLString argVict = args.getOneArgument();
    DLString argOrder = args;

    if (!pch->getClan( )->isRecruiter( pch )) {
        pch->pecho( "Твоих полномочий недостаточно." );
        return;
    }
    
    if (!( victim = PCharacterManager::find( argVict ) )) {
        pch->pecho( "Никого нет с таким именем. Укажи имя полностью." );
        return;
    }

    if (victim->getClan( ) != pch->getClan( )) {
        pch->pecho( "Но %s не принадлежит к твоем клану!", victim->getName( ).c_str( ) );
        return;
    }

    ClanOrder::Pointer org = findOrder( argOrder );    
    ClanOrder::Pointer myOrg = findOrder(pch);

    if (!org && !myOrg) {
        pch->pecho( "%1$^O1 указан%1$Gо||а неверно.", &name );
        return;
    }

    ClanOrder::Pointer targetOrg = org ? org : myOrg;

    if (getAttr( victim ) != targetOrg->name) {
        pch->pecho( "%1$s не состоит в эт%2$Gом|ом|ой %2$O6!", 
                    victim->getName( ).c_str( ), &name );
        return;
    }
    
    delAttr( victim );
    pch->pecho( "%1$s покидает %2$O4.", 
                 victim->getName( ).c_str( ), &name );

    if (victim->isOnline( ))
        victim->getPlayer( )->pecho( "%s исключает тебя из %^O2!",
                                     pch->getName( ).c_str( ), &name );
    else
        PCharacterManager::saveMemory( victim );
}


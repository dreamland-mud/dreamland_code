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

ClanOrder::Pointer ClanOrgs::findOrder( const DLString &oname ) const
{
    ClanOrder::Pointer null;

    if (oname.size( ) == 0)
        return null;

    const_iterator i = find( oname );
    if (i == end( ))
        return null;

    return i->second;
}

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

void ClanOrgs::doMembers( PCharacter *pch ) const
{
    ostringstream buf;
    ClanOrder::Pointer ord = findOrder( pch );

    if (!ord) {
        pch->pecho( "Ты не состоишь в %O6.", &name );
        return;
    }
    
    buf << "\n\r{WИмя         раса        класс         уровень   звание{x\n\r";
    
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    for (PCharacterMemoryList::const_iterator pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;

        if (pcm->getClan( ) != pch->getClan( )
            || pcm->getLevel( ) >= LEVEL_IMMORTAL
            || getAttr( pcm ) != ord->name)
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


void ClanOrgs::doSelfInduct( PCharacter *pch, DLString &arg ) const
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

void ClanOrgs::doInduct( PCharacter *pch, DLString &arg ) const
{
    PCMemoryInterface *victim;

    if (!pch->getClan( )->isRecruiter( pch )) {
        pch->pecho( "Твоих полномочий недостаточно." );
        return;
    }

    if (!hasAttr( pch )) {
        pch->pecho( "Ты не являешься главой %O2.", &name );
        return;
    }
    
    if (!( victim = PCharacterManager::find( arg ) )) {
        pch->pecho( "Никого нет с таким именем. Укажи имя полностью." );
        return;
    }

    if (victim->getClan( ) != pch->getClan( )) {
        pch->pecho( "Но %s не принадлежит к твоему клану!", victim->getName( ).c_str( ) );
        return;
    }
    
    if (hasAttr( victim )) {
        if (getAttr( victim ) != getAttr( pch )) 
            pch->pecho( "%1$s уже состоит в друг%2$Gом|ом|ой %2$O6.", 
                         victim->getName( ).c_str( ), &name );
        else
            pch->pecho( "%1$s и так состоит в тво%2$Gем|ем|ей %2$O6.", 
                         victim->getName( ).c_str( ), &name );
        
        return;
    }
    
    ClanOrder::Pointer ord = findOrder( pch );

    if (!ord) {
        pch->pecho( "%1$^O1, к котор%1$Gому|ому|ой ты принадлежишь, не существует!", &name );
        return;
    }

    if (!ord->canInduct( victim )) {
        pch->pecho( "%s не может вступить в %s.", 
                    victim->getName( ).c_str( ), ord->shortDescr.c_str( ) );
        return;
    }
    
    setAttr( victim, ord->name );
    pch->pecho( "Ты принимаешь %s в %s!", 
                 victim->getName( ).c_str( ), ord->shortDescr.c_str( ) );
    
    if (victim->isOnline( ))
        victim->getPlayer( )->pecho( "%s принимает тебя в %s!",
                                     pch->getName( ).c_str( ),
                                     ord->shortDescr.c_str( ) );
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

void ClanOrgs::doRemove( PCharacter *pch, DLString &arg ) const
{
    PCMemoryInterface *victim;

    if (!pch->getClan( )->isRecruiter( pch )) {
        pch->pecho( "Твоих полномочий недостаточно." );
        return;
    }
    
    if (!hasAttr( pch )) {
        pch->pecho( "Ты не являешься главой %1$O2.", &name );
        return;
    }
    
    if (!( victim = PCharacterManager::find( arg ) )) {
        pch->pecho( "Никого нет с таким именем. Укажи имя полностью." );
        return;
    }

    if (victim->getClan( ) != pch->getClan( )) {
        pch->pecho( "Но %s не принадлежит к твоем клану!", victim->getName( ).c_str( ) );
        return;
    }

    if (getAttr( victim ) != getAttr( pch )) {
        pch->pecho( "%1$s не состоит в тво%2$Gем|ем|ей %2$O6!", 
                    victim->getName( ).c_str( ), &name );
        return;
    }
    
    delAttr( victim );
    pch->pecho( "%1$s покидает тво%2$Gй|й|ю %2$O4.", 
                 victim->getName( ).c_str( ), &name );

    if (victim->isOnline( ))
        victim->getPlayer( )->pecho( "%s исключает тебя из %^O2!",
                                     pch->getName( ).c_str( ), &name );
    else
        PCharacterManager::saveMemory( victim );
}


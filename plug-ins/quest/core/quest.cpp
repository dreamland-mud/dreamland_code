/* $Id: quest.cpp,v 1.1.4.13.6.7 2010-09-01 21:20:46 rufina Exp $
 *
 * ruffina, 2003
 */

#include <stdarg.h>
#include <string.h>

#include "quest.h"
#include "questexceptions.h"
#include "xmlattributequestdata.h"

#include "player_utils.h"

#include "schedulertask.h"

#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "dlscheduler.h"

#include "handler.h"
#include "wiznet.h"
#include "merc.h"
#include "act.h"
#include "def.h"

Quest::Quest( ) 
{
}

bool Quest::help( PCharacter *ch, NPCharacter *questman ) 
{
    return false;
}

void Quest::helpMessage( ostringstream &buf )
{
    buf << "Тебе необходимо следовать по следующему пути: ";
}

Room * Quest::helpLocation( )
{
    return NULL;
}

void Quest::shortInfo( std::ostream &, PCharacter * )
{
}

void Quest::wiznet( const char *status, const char *format, ... ) 
{
    std::basic_ostringstream<char> buf;

    buf << getName( ) << " " << status << ": " << charName;
    
    if (format) {
        va_list ap;
    
        va_start( ap, format );
        buf << ": " << vfmt(0, format, ap);
        va_end( ap );
    }

    ::wiznet( WIZ_QUEST, 0, 0, buf.str().c_str() );
}

int Quest::getAccidentTime( PCMemoryInterface *pci )
{
    return number_range( 2, 5 );
}

int Quest::getPunishTime( PCMemoryInterface *pci )
{
    if (PlayerUtils::isNewbie( pci ))
        return number_range( 15, 20 );
    else
        return number_range( 20, 30 );
}
        
int Quest::getDeathTime( PCMemoryInterface *pci )
{
    if (PlayerUtils::isNewbie( pci ))
        return number_range( 15, 20 );
    else 
        return number_range( 20, 30 );
}            

int Quest::getFailTime( PCMemoryInterface *pci )
{
    if (PlayerUtils::isNewbie( pci ))
        return number_range( 15, 20 );
    else 
        return number_range( 25, 35 );
}

int Quest::getNextTime( PCMemoryInterface *pci )
{
    return number_range( 1, 3 );
}

int Quest::getCancelTime( PCMemoryInterface *pci )
{
    return number_range( 2, 5 );
}

void Quest::setTime( PCMemoryInterface *pcm, int time )
{
    pcm->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->setTime( time );
}

int Quest::getTime( PCMemoryInterface *pcm )
{
    return pcm->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getTime( );
}

class QuestDestroyTask : public SchedulerTask {
public:
    typedef ::Pointer<QuestDestroyTask> Pointer;
    
    QuestDestroyTask( const DLString &name ) : charName( name ) 
    {
    }
    virtual void run( )
    {
        PCMemoryInterface *pci = PCharacterManager::find( charName );
        
        if (pci) 
            pci->getAttributes( ).eraseAttribute( "quest" );
    }
    virtual int getPriority( ) const
    {
        return SCDP_FINAL;
    }

private:
    DLString charName;
};

void Quest::scheduleDestroy( )
{
    DLScheduler::getThis( )->putTaskNOW( 
            QuestDestroyTask::Pointer( NEW, charName ) );
}

Character * Quest::getActor( Character *ch ) 
{
    if (!ch)
        return ch;

    if (ch->is_mirror( ) && ch->doppel)
        return getActor( ch->doppel );
    
    if (!ch->is_npc( ) && !IS_CHARMED(ch))
        return ch;
    
    if (ch->leader && ch->leader != ch)
        return getActor( ch->leader );
    
    if (ch->master && ch->master != ch)
        return getActor( ch->master );
    
    return ch;
}

PCharacter * Quest::getHeroWorld( )
{
    PCMemoryInterface *pci = PCharacterManager::find( charName );               
    
    if (!pci)
        return NULL;
    else
        return pci->getPlayer( );
}

PCMemoryInterface * Quest::getHeroMemory( )
{
    return PCharacterManager::find( charName );
}

bool Quest::hasPartialRewards() const
{
    return false;
}

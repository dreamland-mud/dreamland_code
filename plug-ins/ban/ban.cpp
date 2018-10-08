/* $Id: ban.cpp,v 1.1.2.10 2014-09-19 11:36:01 rufina Exp $
 *
 * ruffina, 2005
 */

#include <fstream>
#include <algorithm>

#include "dl_match.h"
#ifndef __MINGW32__
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif

#include <algorithm>

#include "ban.h"

#include "logstream.h"
#include "dlscheduler.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "def.h"

/*------------------------------------------------------------------------
 * Ban
 *-----------------------------------------------------------------------*/
Ban::Ban( ) : flags( BAN_ALL, &ban_flags )
{
}

bool Ban::match( const DLString &str ) const
{
    return dl_match(pattern.getValue( ).c_str( ), str.c_str( ), true);
}

/*------------------------------------------------------------------------
 * BanManager
 *-----------------------------------------------------------------------*/
BanManager *banManager = 0;
const DLString BanManager::FILE_NAME = "ban.xml";
const DLString BanManager::NODE_NAME = "banlist";

BanManager::BanManager( ) 
                : banFile( DLFile( dreamland->getDbDir( ), FILE_NAME ).getPath( ),
                           NODE_NAME,
                           this )
{
    checkDuplicate( banManager );
    banManager = this;
    DLScheduler::getThis( )->putTaskInSecond( Pointer( this ) );
}

BanManager::~BanManager()
{
    banManager = NULL;
}

void BanManager::initialization( )
{
    if (banFile.load( ))
        LogStream::sendNotice( ) << "Loaded " << size( ) << " ban patterns..." << endl;

    SchedulerTaskRoundPlugin::initialization( );
}

void BanManager::destruction( )
{
    banFile.save( );
    SchedulerTaskRoundPlugin::destruction( );
}

bool BanManager::checkExpire( const Ban &b )
{
    time_t exp = b.expire.getTime( );

    if (exp == 0)
        return false;

    return exp < dreamland->getCurrentTime( );
}
    
void BanManager::run( )
{
    iterator i = std::remove_if(
                     begin( ), 
                     end( ), 
                     std::ptr_fun( &checkExpire ) );

    if (i != end( )) {
        erase( i, end( ) );
        save( );
    }
}

void BanManager::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( 60,  Pointer( this ) );
}

int BanManager::getPriority( ) const
{
    return SCDP_INITIAL;
}
    
void BanManager::save( ) const
{
    banFile.save( );
}

bool BanManager::set(const DLString &patt, int flags, const Date &exp)
{
    iterator i;
    
    Ban b;
    b.pattern.setValue(patt);
    b.flags.setValue(flags);
    b.expire = exp;

    for(i = begin(); i != end(); i++)
        if(i->pattern.getValue() == patt)
            *i = b;
    
    if(i == end())
        push_back(b);

    save( );
    
    return true;
}

bool BanManager::del(const DLString &patt)
{
    iterator i;
    
    for(i = begin(); i != end(); i++)
        if(i->pattern.getValue() == patt) {
            erase(i);
            return true;
        }
    
    save( );
    return false;
}

bool BanManager::del(int idx)
{
    if(idx < 0 || idx >= (int)size())
        return false;
    
    erase(begin() + idx);

    save( );
    return true;
}

bool BanManager::check( const DLString &host, int flags ) const
{
    const_iterator i;
    
    for (i = begin(); i != end(); i++)
        if (i->flags.getValue( ) == flags && i->match(host)) 
            return true;

    return false;
}
    
bool BanManager::check( Descriptor *d, int flags ) const
{
    ViaVector::iterator i;
    
    if (check(d->host, flags))
        return true;
    
    if (check(d->realip, flags))
        return true;
    
    for (i = d->via.begin(); i != d->via.end(); i++) {
        if (check(inet_ntoa(i->first), flags))
            return true;
        
        if (check(i->second.c_str(), flags))
            return true;
    }

    return false;
}

bool BanManager::checkVerbose( Descriptor *d, int flags ) const
{
    if (!check( d, flags ))
        return false;

    switch (flags) {
    case BAN_ALL:
        d->send("Your site has been banned from this mud.\n\r");
        break;

    case BAN_NEWBIES:
        d->send("New players are not allowed from your site.\n\r");
        break;

    case BAN_PLAYER:
        d->send("Your site has been banned for players.\n\r");
        break;

    case BAN_CONFIRM:
        d->send("Players from your site cannot request confirmation.\n\r");
        break;
    }

    return true;
}


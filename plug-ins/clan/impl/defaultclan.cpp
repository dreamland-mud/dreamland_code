/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultclan.h"
#include "clanmanager.h"

#include "class.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"

#include "act.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

/*-----------------------------------------------------------------
 * DefaultClan 
 *----------------------------------------------------------------*/
DefaultClan::DefaultClan( )
            : channel( false ), dispersed( true ), diplomacy( false ),
              hidden( false )
{
}


ClanData * DefaultClan::getData( ) 
{
    return data.getPointer( );
}

const ClanMembership * DefaultClan::getMembership( ) const
{
    return membership.getPointer( );
}

ClanMembership * DefaultClan::getMembership( ) 
{
    return membership.getPointer( );
}

const ClanTitles * DefaultClan::getTitles( ) const
{
    return titles.getPointer( );
}

const ClanOrgs * DefaultClan::getOrgs( ) const
{
    return orgs.getPointer( );
}

const DLString & DefaultClan::getTitle( PCMemoryInterface *pcm ) const
{
    if (orgs) {
        ClanOrder::Pointer ord = orgs->findOrder( pcm );
        
        if (ord) {
            const DLString &title = ord->getTitle( pcm );
            if (title.size( ) != 0)
                return title;
        }
    }
    
    if (titles) {
        return titles->build( pcm );
    }

    return DLString::emptyString;
}

bool DefaultClan::isLeader( PCMemoryInterface *pcm ) const
{
    if (leader.getValue( ) < 0)
        return false;
        
    return pcm->getClanLevel( ) >= leader.getValue( );
}

bool DefaultClan::isRecruiter( PCMemoryInterface *pcm ) const
{
    if (recruiter.getValue( ) < 0)
        return false;
        
    return pcm->getClanLevel( ) >= recruiter.getValue( );
}

bool DefaultClan::canInduct( PCharacter *ch ) const
{
    return induct.allow( ch );
}

void DefaultClan::makeMonument( Character *ch, Character *killer ) const
{
    Object *obj;

    if (!killer || ch == killer)
        return;

    if (ch->is_npc( ) || killer->is_npc( ))
        return;
    
    obj = create_object( get_obj_index( OBJ_VNUM_MONUMENT ), 0 );
    obj->timer = 24 * 24 * 2; // 48 real life hour
    
    obj->setDescription( 
            fmt( NULL, monument.getValue( ).c_str( ), ch, killer ).c_str( ) );

    obj->fmtName( 
            obj->getName( ), ch->getNameP( ), killer->getNameP( ) );

    obj->fmtShortDescr( 
            obj->getShortDescr( ), ch->getNameP( '3' ).c_str( ), killer->getNameP( '2').c_str( ) );

    obj_to_room( obj, ch->in_room );
}

bool DefaultClan::isEnemy( const Clan &clan ) 
{
    XMLVectorBase<XMLClanReference>::iterator i;

    for (i = enemies.begin( ); i != enemies.end( ); i++) {
        if (*i == clan)
            return true;
    }
    
    return false;
}

void DefaultClan::handleVictory( PCharacter *ch, PCharacter *victim )
{
    if (ch == victim || ch->is_immortal( ) || victim->is_immortal( ))
        return;

    if (!data)
        return;
        
    data->victory[ch->getRealLevel( ) / 20]++;
    data->save( );
}

void DefaultClan::handleDefeat( PCharacter *ch, PCharacter *killer )
{
    if (ch == killer || ch->is_immortal( ) || killer->is_immortal( ))
        return;

    if (!data)
        return;

    data->defeat[ch->getRealLevel( ) / 20]++;
    data->save( );
}

bool DefaultClan::isHidden( ) const
{
    return hidden.getValue( );
}
bool DefaultClan::isValid( ) const
{
    return true;
}
const DLString & DefaultClan::getName( ) const
{
    return Clan::getName( );
}
void DefaultClan::setName( const DLString &name )
{
    this->name = name;
}
const DLString &DefaultClan::getShortName( ) const
{
    return shortName.getValue( );
}
const DLString &DefaultClan::getLongName( ) const
{
    return longName.getValue( );
}
const DLString &DefaultClan::getColor( ) const
{
    return color.getValue( );
}
const DLString &DefaultClan::getPaddedName( ) const
{
    return padName.getValue( );
}
bool DefaultClan::isDispersed( ) const
{
    return dispersed.getValue( );
}
int DefaultClan::getRecallVnum( ) const
{
    return recallVnum.getValue( );
}
bool DefaultClan::hasChannel( ) const
{
    return channel.getValue( );
}
bool DefaultClan::hasDiplomacy( ) const
{
    return diplomacy.getValue( );
}

void DefaultClan::loaded( )
{
    clanManager->registrate( Pointer( this ) );

    data = ClanData::Pointer( NEW, getName( ) );
    data->load( );
}

void DefaultClan::unloaded( )
{
    clanManager->unregistrate( Pointer( this ) );
}


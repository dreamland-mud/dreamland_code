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
#include "loadsave.h"
#include "string_utils.h"
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


void DefaultClan::getCastMessages( StringList &self, StringList &vict, StringList &room ) const
{
    self = msgSelf.toList( );
    vict = msgVict.toList( );
    room = msgRoom.toList( );
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

const DLString & DefaultClan::getTitle( PCMemoryInterface *pcm, lang_t lang ) const
{
    if (orgs) {
        ClanOrder::Pointer ord = orgs->findOrder( pcm );

        if (ord) {
            const DLString &title = ord->getTitle( pcm, lang );
            if (title.size( ) != 0)
                return title;
        }
    }

    if (titles) {
        return titles->build( pcm, lang );
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
    
    // The clan's own monument text and keyword blob are single-language fields, so
    // the description stays Russian until they become multi-language.
    obj->setDescription(
            fmt( NULL, monument.getValue( ).c_str( ), ch, killer ), LANG_DEFAULT );

    // Prototype 105 templates the victim and the killer into its short descr and
    // its English keyword in all three languages. Writing only LANG_DEFAULT left
    // an EN reader looking at the raw "a monument to %s from %s", and flattened
    // every language's keywords into the Russian slot. Each slot now carries its
    // own language; the clan's mixed-alphabet keyword blob rides along on Russian,
    // which is where it used to land, and stays matchable from any language
    // because keyword lookup scans every slot.
    for (int l = LANG_MIN; l < LANG_MAX; l++) {
        lang_t lang = (lang_t)l;
        DLString victimName = ch->getNameP( '3', lang );
        DLString killerName = killer->getNameP( '2', lang );

        const DLString &shortPattern = obj->getShortDescr(lang);
        if (!shortPattern.empty())
            obj->setShortDescr( fmt(0, shortPattern.c_str(),
                                    victimName.c_str(), killerName.c_str()), lang );

        DLString victimKey = ch->getNameP( '1', lang );
        DLString killerKey = killer->getNameP( '1', lang );
        DLString kw = obj->getKeyword(lang);

        if (String::contains(kw, "%"))
            kw = fmt(0, kw.c_str(), victimKey.c_str(), killerKey.c_str());
        else
            kw = kw + DLString::SPACE + victimKey + DLString::SPACE + killerKey;

        if (lang == LANG_DEFAULT && !monumentName.getValue( ).empty( ))
            kw = monumentName.getValue( ) + DLString::SPACE + kw;

        obj->setKeyword( kw, lang );
    }

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
const DLString &DefaultClan::getRussianName( ) const
{
    return nameRus;
}
const DLString &DefaultClan::getUkrainianName( ) const
{
    return nameUa;
}
const DLString &DefaultClan::getEnglishName( ) const
{
    return nameEn;
}
const DLString &DefaultClan::getShortFor( lang_t lang ) const
{
    if (lang == LANG_EN && !shortEn.getValue( ).empty( ))
        return shortEn.getValue( );
    if (lang == LANG_UA && !shortUa.getValue( ).empty( ))
        return shortUa.getValue( );
    if (!shortRus.getValue( ).empty( ))
        return shortRus.getValue( );

    // Nothing authored: the identity key beats an empty column.
    return getName( );
}
const DLString &DefaultClan::getShortName( ) const
{
    return shortName.getValue( );
}
const DLString &DefaultClan::getLongName( ) const
{
    return longName.getValue( );
}
const DLString &DefaultClan::getLongNameFor( lang_t lang ) const
{
    if (lang == LANG_EN && !longNameEn.getValue( ).empty( ))
        return longNameEn.getValue( );
    if (lang == LANG_UA && !longNameUa.getValue( ).empty( ))
        return longNameUa.getValue( );

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

const DLString& DefaultClan::getChannelPattern() const
{
    return channelPattern;
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


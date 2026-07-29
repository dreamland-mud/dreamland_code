/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermemory.cpp  -  description
                             -------------------
    begin                : Tue May 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "pcharactermemory.h"
#include "grammar_entities_impl.h"
#include "character.h"
#include "string_utils.h"
#include "lang.h"
#include "merc.h"
#include "def.h"

PCharacterMemory::PCharacterMemory( )
                   : ethos( ETHOS_NULL, &ethos_table )
{
}

PCharacterMemory::~PCharacterMemory( )
{
}

const DLString& PCharacterMemory::getName( ) const 
{
        return name.getValue( );
}

const DLString &PCharacterMemory::getNameP(char gram_case) const
{
    if (russianName.getFullForm().empty())
        return name;

    return russianName.decline(gram_case);
}

/**
 * Per-language name of an offline player.
 *
 * Without this override the interface default (CharacterMemoryInterface) throws
 * the language away and answers with the Russian declension for everyone, so an
 * offline character's name leaked Russian into English and Ukrainian output --
 * clan rosters, note and board authors, and the channel servlet, which renders a
 * message from an offline player through a dummy PCharacter built by setMemory().
 *
 * The fallbacks match PCharacter's cached noun: English is undeclined and falls
 * back to the login (which is what autofill romanises it from anyway), Ukrainian
 * falls back to the Russian pad.
 */
const DLString &PCharacterMemory::getNameP(char gram_case, lang_t lang) const
{
    if (lang == LANG_EN) {
        if (englishName.getValue( ).empty( ))
            return name;

        return englishName;
    }

    if (lang == LANG_UA && !ukrainianName.getFullForm( ).empty( ))
        return ukrainianName.decline( gram_case );

    return getNameP( gram_case );
}

const char * PCharacterMemory::getNameC() const
{
    return name.c_str();
}

void PCharacterMemory::setName( const DLString& name ) 
{
        this->name.setValue( name );
}

const DLString& PCharacterMemory::getPassword( ) const 
{
        return password.getValue( );
}
void PCharacterMemory::setPassword( const DLString &password ) 
{
        this->password = password;
}

const Date& PCharacterMemory::getLastAccessTime( ) const 
{
        return lastAccessTime;
}

void PCharacterMemory::setLastAccessTime( const Date& lastAccessTime ) 
{
        this->lastAccessTime = lastAccessTime;
}

const DLString& PCharacterMemory::getLastAccessHost( ) const 
{
        return lastAccessHost;
}

void PCharacterMemory::setLastAccessHost( const DLString& lastAccessHost ) 
{
        this->lastAccessHost.setValue( lastAccessHost );
}

void PCharacterMemory::setDescription( const DLString& d, lang_t lang )
{
    description[lang] = d;
}

const DLString & PCharacterMemory::getDescription( lang_t lang ) const
{
    return description.get(lang);
}

const XMLMultiString & PCharacterMemory::getDescription( ) const
{
    return description;
}

void PCharacterMemory::setDescription( const XMLMultiString &description )
{
    this->description = description;
} 

const DLString& PCharacterMemory::getPretitle( ) const 
{
    return pretitle.getValue( );
}

void PCharacterMemory::setPretitle( const DLString& pretitle ) 
{
    this->pretitle.setValue( pretitle );
}

const DLString& PCharacterMemory::getRussianPretitle( ) const 
{
    return russianPretitle.getValue( );
}

void PCharacterMemory::setRussianPretitle( const DLString& pretitle ) 
{
    this->russianPretitle.setValue( pretitle );
}

void PCharacterMemory::setTitle( const DLString &title )
{
    this->title = title;
}

const DLString & PCharacterMemory::getTitle( ) const 
{
    return title.getValue( );
}

short PCharacterMemory::getLevel( ) const 
{
        return level.getValue( );
}

void PCharacterMemory::setLevel( short level ) 
{
        this->level = level;
}

int PCharacterMemory::getTrust( ) const 
{
        return trust.getValue( );
}

void PCharacterMemory::setTrust( int trust ) 
{
        this->trust = trust;;
}
int PCharacterMemory::getSecurity( ) const 
{
        return security.getValue( );
}

void PCharacterMemory::setSecurity( int security ) 
{
        this->security = security;
}

int PCharacterMemory::getQuestPoints( ) const 
{
        return questpoints;
}

void PCharacterMemory::setQuestPoints( int questpoints ) 
{
        this->questpoints = questpoints;
}

ClanReference &PCharacterMemory::getPetition( ) 
{
        return petition;
}

void PCharacterMemory::setPetition( const ClanReference & petition ) 
{
        this->petition.assign( petition );
}

ClanReference &PCharacterMemory::getClan( ) 
{
        return clan;
}

void PCharacterMemory::setClan( const ClanReference & clan ) 
{
        this->clan.assign( clan );
}

HometownReference &PCharacterMemory::getHometown( ) 
{
        return hometown;
}

void PCharacterMemory::setHometown( const HometownReference & hometown ) 
{
        this->hometown.assign( hometown );
}

ProfessionReference &PCharacterMemory::getProfession( ) 
{
        return profession;
}

void PCharacterMemory::setProfession( const ProfessionReference & profession ) 
{
        this->profession.assign( profession );
}

RaceReference &PCharacterMemory::getRace( ) 
{
        return race;
}

void PCharacterMemory::setRace( const RaceReference & race ) 
{
        this->race.assign( race );
}

short PCharacterMemory::getClanLevel( ) const 
{
        return clanLevel.getValue( );
}

void PCharacterMemory::setClanLevel( short clanLevel ) 
{
        this->clanLevel.setValue( clanLevel );
}

short PCharacterMemory::getSex( ) const 
{
        return sex.getValue( );
}

void PCharacterMemory::setSex( short sex ) 
{
        this->sex.setValue( sex );
}

int PCharacterMemory::getAlignment( ) const
{
        return alignment;
}

void PCharacterMemory::setAlignment( int value )
{
        alignment = value;
}

int PCharacterMemory::getEthos( ) const
{
        return ethos;
}

void PCharacterMemory::setEthos( int value )
{
        ethos = value;
}


XMLAttributes& PCharacterMemory::getAttributes( ) 
{
        return attributes;
}

const XMLAttributes& PCharacterMemory::getAttributes( ) const 
{
        return attributes;
}

void PCharacterMemory::setAttributes( const XMLAttributes& attributes ) 
{
        this->attributes = attributes;
}

Remorts & PCharacterMemory::getRemorts( ) 
{
    return remorts;
}

void PCharacterMemory::setRemorts( const Remorts& remorts )  
{
    this->remorts = remorts;
}

const InflectedString& PCharacterMemory::getRussianName( ) const 
{
    return russianName;
}

void PCharacterMemory::setRussianName( const DLString& name )
{
    russianName.setFullForm( name );
}

const DLString& PCharacterMemory::getEnglishName( ) const
{
    return englishName.getValue( );
}

void PCharacterMemory::setEnglishName( const DLString& name )
{
    englishName.setValue( name );
}

const InflectedString& PCharacterMemory::getUkrainianName( ) const
{
    return ukrainianName;
}

void PCharacterMemory::setUkrainianName( const DLString& name )
{
    ukrainianName.setFullForm( name );
}

lang_t PCharacterMemory::getBaseLang( ) const
{
    int v = baseLang.getValue( );

    if (v >= 1 && v <= LANG_MAX)
        return (lang_t)(v - 1);

    // Same fallback as PCharacter: characters predating the field have nothing
    // stored, so infer from the login's script.
    return String::hasCyrillic( getName( ) ) ? LANG_RU : LANG_EN;
}

void PCharacterMemory::setBaseLang( lang_t lang )
{
    if (lang >= LANG_MIN && lang < LANG_MAX)
        baseLang.setValue( (int)lang + 1 );
}

int PCharacterMemory::get_trust( ) const
{
    if (getAttributes( ).isAvailable( "coder" ))
        return 0xFFFF;
    
    if (getTrust( ) != 0)
        return getTrust( );

    return getLevel( );
}

ReligionReference & PCharacterMemory::getReligion( ) 
{
    return religion;
}

void PCharacterMemory::setReligion( const ReligionReference &religion )
{
    this->religion.assign( religion );
}

bool PCharacterMemory::isOnline( ) const
{
    return false;
}

PCharacter * PCharacterMemory::getPlayer( ) 
{
    return NULL;
}

NPCharacter * PCharacterMemory::getMobile( ) 
{
    return NULL;
}

PCSkills & PCharacterMemory::getSkills()
{
    return skills;        
}

void PCharacterMemory::setSkills(const PCSkills &skills)
{
    this->skills = skills;        
}

PCBonuses & PCharacterMemory::getBonuses()
{
    return bonuses;        
}

void PCharacterMemory::setBonuses(const PCBonuses &bonuses)
{
    this->bonuses = bonuses;        
}

int PCharacterMemory::getStartRoom() const
{
    return start_room;
}

void PCharacterMemory::setStartRoom(int vnum)
{
    this->start_room = vnum;        
}

int PCharacterMemory::getLoyalty() const
{
    return loyalty;
}

void PCharacterMemory::setLoyalty(int value)
{
    this->loyalty = value;        
}

using namespace Grammar;

Noun::Pointer PCharacterMemory::toNoun( const DLObject *forWhom, int flags ) const
{
    const Character *wch = dynamic_cast<const Character *>(forWhom);
    MultiGender mg( getSex( ), Number::SINGULAR );

    DLString rname = getRussianName( ).getFullForm( );
    if (rname.empty( ))
        rname = getName( );

    if (wch && viewerLang( wch ) == LANG_EN)
        return InflectedString::Pointer( NEW, getName( ), mg );
    else
        return InflectedString::Pointer( NEW, rname, mg );
}

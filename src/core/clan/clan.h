/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLAN_H__
#define __CLAN_H__

#include "globalregistryelement.h"
#include "stringlist.h"
#include "lang.h"

class Character;
class PCharacter;
class Object;
class PCMemoryInterface; 

class ClanData;
class ClanMembership;
class ClanTitles;
class ClanOrgs;

class Clan : public GlobalRegistryElement {
public:
    typedef ::Pointer<Clan> Pointer;
    
    Clan( );
    Clan( const DLString & );
    virtual ~Clan( );

    virtual const DLString &getName( ) const;
    virtual bool isValid( ) const;

    virtual const DLString &getShortName( ) const;
    virtual const DLString &getLongName( ) const;
    virtual const DLString &getColor( ) const;
    virtual const DLString &getPaddedName( ) const;
    virtual const DLString &getChannelPattern( ) const;

    /** A clan answers to two display names in each language: the ceremonial one
     *  ("Invaders Cabal", declinable in ru/ua) and the short one an audience
     *  already knows it by ("Invaders"). Neither is getName(), which is the
     *  identity key that pfiles and diplomacy maps are written in.
     *  getRussianName()/getUkrainianName() come from GlobalRegistryElement and
     *  hold the ceremonial forms; English needs its own accessor because a
     *  registry element's getName() is normally the English name, and here it
     *  is not. Both selectors fall back to Russian rather than to nothing. */
    virtual const DLString &getEnglishName( ) const;
    virtual const DLString &getNameFor( lang_t ) const;
    virtual const DLString &getShortFor( lang_t ) const;

    /** The descriptive line `clan list` puts beside the name ("Defenders of Good
     *  and Light"). Carries a {hh help link, whose vnum is the same in every
     *  language -- only the anchor text is translated. */
    virtual const DLString &getLongNameFor( lang_t ) const;
    
    /** Cast flavour shown when a member casts one of this clan's own spells.
     *  Three index-parallel lists; all empty means fall back to the skill group. */
    virtual void getCastMessages( StringList &self, StringList &vict, StringList &room ) const;

    virtual const DLString & getTitle( PCMemoryInterface * ) const;
    virtual bool isLeader( PCMemoryInterface * ) const;
    virtual bool isRecruiter( PCMemoryInterface * ) const;
    virtual bool canInduct( PCharacter * ) const;
    virtual void onInduct(PCharacter *) const;
    
    virtual void makeMonument( Character *, Character * ) const;
    virtual void handleVictory( PCharacter *, PCharacter * );
    virtual void handleDefeat( PCharacter *, PCharacter * );

    virtual bool isEnemy( const Clan & );
    virtual bool isDispersed( ) const;
    virtual int getRecallVnum( ) const;
    virtual bool hasChannel( ) const;
    virtual bool hasDiplomacy( ) const;
    virtual bool isHidden( ) const;

    virtual ClanData * getData( );
    virtual const ClanMembership * getMembership( ) const;
    virtual ClanMembership * getMembership( );
    virtual const ClanTitles * getTitles( ) const;
    virtual const ClanOrgs * getOrgs( ) const;

protected:
    DLString name;
};

#endif

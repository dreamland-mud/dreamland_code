/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharacter.h  -  description
                             -------------------
    begin                : Thu May 3 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef PCHARACTER_H
#define PCHARACTER_H

#include "xmlstring.h"
#include "xmllong.h"
#include "xmldate.h"
#include "xmlflags.h"
#include "xmlglobalbitvector.h"
#include "xmlglobalarray.h"
#include "xmltimestamp.h"

#include "xmlrussianstring.h"

#include "pcmemoryinterface.h"
#include "xmlattributes.h"
#include "character.h"
#include "remortdata.h"
#include "pcskilldata.h"
#include "hometown.h"

class PCharacterMemory;

/*
 * PlayerAge
 */
class PlayerAge {
public:
    PlayerAge( );
    
    void clear( );
    int getTime( ) const;
    int getTrueTime( ) const;
    int getYears( ) const;
    int getTrueYears( ) const;
    int getHours( ) const;
    int getTrueHours( ) const;
    void modifyYears( int );
    void setTruePlayed( int );
    Date getLogon( ) const;
    void setLogon( time_t );
    
    static const int BASE_AGE;
    static const int HOURS_PER_YEAR;
protected:
    time_t logon;
    int played;
    int true_played;
};

/*
 * XMLPlayerAge
 */
class XMLPlayerAge : public PlayerAge {
public:
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& ) throw( ExceptionBadType );
};

/*
 * CachedNoun
 */
struct CachedNoun {
    void update( PCharacter * );
    void clear( );

    RussianString::Pointer name;
    RussianString::Pointer russian;
    RussianString::Pointer vampire;
    RussianString::Pointer vampire2;
    RussianString::Pointer immortal;
    RussianString::Pointer pretitle;
    RussianString::Pointer pretitleRussian;
};

/**
 * @short Data which only PC's have.
 * @author Igor S. Petrenko
 */
class PCharacter : public Character, public PCMemoryInterface
{
XML_OBJECT;
public:	
    typedef ::Pointer<PCharacter> Pointer;

public:
    PCharacter( );
    virtual ~PCharacter( );

    // recycle
    virtual void init( );

    // gate to pc/npc info
    virtual PCharacter *getPC();
    virtual NPCharacter *getNPC();
    virtual const PCharacter *getPC( ) const;
    virtual const NPCharacter *getNPC( ) const;
    virtual bool is_npc( ) const;
    
    // pc memory update 
    PCharacterMemory* getMemory( );
    void setMemory( PCharacterMemory* );

    // work with profiles (implemented in anatolia_core)
    void save( );
    void load( );
    
    // xml container
    virtual bool nodeFromXML( const XMLNode::Pointer & );
 
    // set-get methods inherited from PCMemoryInterface
    virtual bool isOnline( ) const;
    virtual PCharacter * getPlayer( );

    virtual const DLString& getName( ) const throw( );

    virtual const DLString& getPassword( ) const throw( );
    virtual void setPassword( const DLString& ) throw( );

    virtual ClanReference &getClan( ) throw( );
    virtual void setClan( const ClanReference & ) throw( );

    virtual ClanReference &getPetition( ) throw( );
    virtual void setPetition( const ClanReference & ) throw( );
    
    virtual short getClanLevel( ) const throw( );
    virtual void setClanLevel( short clanLevel ) throw( );

    virtual ProfessionReference & getTrueProfession( );
    ProfessionReference & getSubProfession( );
    void setSubProfession( const ProfessionReference & );

    virtual HometownReference &getHometown( ) throw( );
    virtual void setHometown( const HometownReference & ) throw( );

    virtual const Date& getLastAccessTime( ) const throw( );
    virtual void setLastAccessTime( const Date& ) throw( );
    void setLastAccessTime( );

    virtual const DLString& getLastAccessHost( ) const throw( );
    virtual void setLastAccessHost( const DLString& ) throw( );

    virtual short getLevel( ) const throw( );

    virtual int getTrust( ) const throw( );
    virtual void setTrust( int ) throw( );

    virtual int getSecurity( ) const throw( );
    virtual void setSecurity( int ) throw( );

    virtual short getSex( ) const throw( );

    virtual XMLAttributes& getAttributes( ) throw( );
    virtual void setAttributes( const XMLAttributes& attributes ) throw( );
    virtual const XMLAttributes& getAttributes( ) const throw( );
	    
    virtual const RussianString& getRussianName( ) const throw( );
    virtual void setRussianName( const DLString& ) throw( );

    virtual Remorts& getRemorts( ) throw( );
    virtual const Remorts& getRemorts( ) const throw( );
    virtual void setRemorts( const Remorts& ) throw( ); 

    virtual const DLString& getRussianPretitle( ) const throw( );
    virtual void setRussianPretitle( const DLString& ) throw( );
    
    virtual const DLString& getPretitle( ) const throw( );
    virtual void setPretitle( const DLString& ) throw( );

    // set-get methods inherited from Character
    virtual const GlobalBitvector & getWearloc( );
    virtual void setDescription( const DLString& );
    virtual const char * getDescription( ) const;
    
    // title
    void setTitle( const DLString& );
    const DLString & getTitle( ) const;
    DLString getParsedTitle( );
    
    // name and sex formatting
    inline const char *getNameP( ) const;
    virtual DLString getNameP( char gram_case ) const;
    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;
    virtual void updateCachedNoun( );

    // visibility of things
    bool canSeeLevel( PCharacter * );
    bool canSeeProfession( PCharacter * );
    
    // pc skills
    int skill_points( );
    int skill_points( int );
    virtual int applyCurse( int );
    PCSkillData & getSkillData( int );
    inline PCSkills & getSkills( );
    void updateSkills( );

    // experience 
    int getExpToLevel( );
    int getBaseExp( );
    int getExpPerLevel( int lvl = -1, int remorts = -1 );
    // (implemented in anatolia_core)
    void gainExp( int ); 
    void advanceLevel( );
    
    // trust and immortality
    virtual bool isCoder( ) const;
    virtual int get_trust( ) const;
    virtual bool is_immortal( ) const;
    
    // stats
    virtual int getCurrStat( int );
    int getMaxStat( int );
    int getMaxTrain( int );
    void updateStats( );
    
    // player pk counter (implemented in anatolia_core)
    void check_hit_newbie( Character *victim );

    // misc
    virtual bool is_vampire( ) const;
    virtual bool is_mirror( ) const;
    virtual short getModifyLevel( ) const;

    // configuration
    virtual PlayerConfig::Pointer getConfig( ) const;

private:
    XML_VARIABLE XMLString password; 
    XML_VARIABLE XMLDate lastAccessTime;
    XML_VARIABLE XMLString lastAccessHost;
    XML_VARIABLE XMLClanReference petition;
    XML_VARIABLE XMLShort clanLevel;
    XML_VARIABLE XMLHometownReference   hometown;
    XML_VARIABLE XMLProfessionReference subprofession;

    XML_VARIABLE XMLAttributes attributes;
    XML_VARIABLE Remorts remorts;
    XML_VARIABLE XMLIntegerNoEmpty trust;
    XML_VARIABLE XMLRussianString russianName;

    XML_VARIABLE XMLStringNoEmpty title;
    XML_VARIABLE XMLStringNoEmpty pretitle;
    XML_VARIABLE XMLStringNoEmpty russianPretitle;
    XML_VARIABLE XMLString description;
    XML_VARIABLE PCSkills skills;
    XML_VARIABLE XMLInteger security;
    XML_VARIABLE XMLIntegerNoEmpty newbie_hit_counter; 
    
    CachedNoun cachedNoun;

public:
    XML_VARIABLE XMLGlobalBitvector wearloc;
    XML_VARIABLE XMLGlobalArray      desires;

    // wizard stuff
    XML_VARIABLE XMLStringNoEmpty bamfin;
    XML_VARIABLE XMLStringNoEmpty bamfout;
    XML_VARIABLE XMLIntegerNoEmpty wiznet;
    
    // age and played hours
    XML_VARIABLE XMLInteger last_level;
    XML_VARIABLE XMLTimeStamp last_logoff;
    XML_VARIABLE XMLPlayerAge age;

    // PK/death timers and counters
    XML_VARIABLE XMLInteger		last_death_time;
    XML_VARIABLE XMLIntegerNoEmpty	ghost_time;
    XML_VARIABLE XMLIntegerNoEmpty	PK_time_v;
    XML_VARIABLE XMLIntegerNoEmpty	PK_time_sk;
    XML_VARIABLE XMLIntegerNoEmpty	PK_time_t;
    XML_VARIABLE XMLIntegerNoEmpty	PK_flag;	/* KILLER, SLAIN, VIOLENT, GHOST, THIEF */
    XML_VARIABLE XMLIntegerNoEmpty	death;
    XML_VARIABLE XMLIntegerNoEmpty	anti_killed;
    XML_VARIABLE XMLIntegerNoEmpty	has_killed;
    
    // player-specific parameters
    XML_VARIABLE XMLInteger		perm_hit;
    XML_VARIABLE XMLInteger		perm_mana;
    XML_VARIABLE XMLInteger		perm_move;
    XML_VARIABLE XMLInteger		max_skill_points;
    XML_VARIABLE XMLIntegerNoEmpty	practice;
    XML_VARIABLE XMLIntegerNoEmpty	train;
    XML_VARIABLE XMLIntegerNoEmpty	loyalty;          // real ethos - dynamic
    XML_VARIABLE XMLInteger		curse;
    XML_VARIABLE XMLInteger		bless;

    // money
    XML_VARIABLE XMLIntegerNoEmpty	bank_s;
    XML_VARIABLE XMLIntegerNoEmpty	bank_g;
    
    // quest
    XML_VARIABLE XMLIntegerNoEmpty     questpoints;	

    NPCharacter *	pet;

    // fields used by skills
    PCharacter *	guarding;
    PCharacter *	guarded_by;
    XML_VARIABLE XMLInteger          shadow;		// INVADER (shadow)
    
    // config
    XML_VARIABLE XMLFlags config;
    bool              	confirm_delete;

    // switch
    NPCharacter		*switchedTo;

    XML_VARIABLE XMLInteger start_room;
};


inline const char * PCharacter::getNameP( ) const 
{ 
    return Character::getNameP( ); 
}

inline PCSkills & PCharacter::getSkills( )
{
    return skills;
}

#endif


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
#include "xmlglobalarray.h"
#include "xmltimestamp.h"
#include "xmlmultistring.h"
#include "xmlinflectedstring.h"

#include "pcmemoryinterface.h"
#include "xmlattributes.h"
#include "character.h"
#include "remortdata.h"
#include "pcskilldata.h"
#include "hometown.h"
#include "bonus.h"

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
    void fromXML( const XMLNode::Pointer& ) ;
};

/*
 * CachedNoun
 */
struct CachedNoun {
    void update( PCharacter * );
    void clear( );

    map<lang_t, InflectedString::Pointer> name;
    map<lang_t, InflectedString::Pointer> vampire;
    map<lang_t, InflectedString::Pointer> vampire2;
    map<lang_t, InflectedString::Pointer> immortal;
    map<lang_t, InflectedString::Pointer> pretitle;
    DLString allForms;
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
    bool load( );
    
    // xml container
    virtual bool nodeFromXML( const XMLNode::Pointer & );
 
    // set-get methods inherited from PCMemoryInterface
    virtual const DLString& getName( ) const ;
    virtual void setName( const DLString& ) ;
    virtual const char * getNameC( ) const;

    virtual bool isOnline( ) const;

    virtual const DLString& getPassword( ) const ;
    virtual void setPassword( const DLString& ) ;

    virtual ClanReference &getClan( ) ;
    virtual void setClan( const ClanReference & ) ;

    virtual ClanReference &getPetition( ) ;
    virtual void setPetition( const ClanReference & ) ;
    
    virtual short getClanLevel( ) const ;
    virtual void setClanLevel( short clanLevel ) ;

    virtual HometownReference &getHometown( ) ;
    virtual void setHometown( const HometownReference & ) ;

    virtual const Date& getLastAccessTime( ) const ;
    virtual void setLastAccessTime( const Date& ) ;
    void setLastAccessTime( );

    virtual const DLString& getLastAccessHost( ) const ;
    virtual void setLastAccessHost( const DLString& ) ;

    virtual short getLevel( ) const ;

    virtual int getTrust( ) const ;
    virtual void setTrust( int ) ;

    virtual int getSecurity( ) const ;
    virtual void setSecurity( int ) ;

    virtual int getQuestPoints( ) const ;
    virtual void setQuestPoints( int ) ;
    int addQuestPoints(int);

    virtual short getSex( ) const ;
    virtual void setSex( short ) ;

    virtual XMLAttributes& getAttributes( ) ;
    virtual void setAttributes( const XMLAttributes& attributes ) ;
    virtual const XMLAttributes& getAttributes( ) const ;
            
    virtual const InflectedString& getRussianName( ) const ;
    virtual void setRussianName( const DLString& ) ;

    virtual Remorts& getRemorts( ) ;
    virtual const Remorts& getRemorts( ) const ;
    virtual void setRemorts( const Remorts& ) ; 

    virtual const DLString& getRussianPretitle( ) const ;
    virtual void setRussianPretitle( const DLString& ) ;
    
    virtual const DLString& getPretitle( ) const ;
    virtual void setPretitle( const DLString& ) ;

    virtual int getStartRoom() const;
    virtual void setStartRoom(int vnum);

    virtual int getLoyalty() const;
    virtual void setLoyalty(int value);

    virtual void setTitle( const DLString& );
    virtual const DLString & getTitle( ) const;
    
    virtual void setDescription( const DLString&, lang_t lang );
    virtual const DLString & getDescription( lang_t lang ) const;
    virtual const XMLMultiString & getDescription( ) const;
    virtual void setDescription( const XMLMultiString & ); 
    
    // name and sex formatting
    virtual const DLString &getNameP( char gram_case ) const;
    virtual NounPointer toNoun( const DLObject *forWhom = NULL, int flags = 0 ) const;
    void updateCachedNoun();

    // visibility of things
    bool canSeeLevel( PCharacter * );
    bool canSeeProfession( PCharacter * );
    
    // pc skills
    PCSkillData & getSkillData( int );
    virtual PCSkills & getSkills();
    virtual void setSkills(const PCSkills &);
    virtual PCBonuses & getBonuses();
    virtual void setBonuses(const PCBonuses &);
    void updateSkills( );

    // experience 
    int getExpToLevel( );
    int getBaseExp( );
    int getExpPerLevel( int lvl = -1, int remorts = -1 );
    
    // trust and immortality
    virtual bool isCoder( ) const;
    virtual int get_trust( ) const;
    virtual bool is_immortal( ) const;
    
    // stats
    virtual int getCurrStat( int );
    int getMaxStat( int );
    int getMaxTrain( int );
    
    // misc
    virtual bool is_vampire( ) const;
    virtual bool is_mirror( ) const;
    virtual short getModifyLevel( ) const;

    // configuration
    virtual PlayerConfig getConfig( ) const;

    // return this player or mob they're switched to
    Character *body();

private:
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLString password; 
    XML_VARIABLE XMLDate lastAccessTime;
    XML_VARIABLE XMLString lastAccessHost;
    XML_VARIABLE XMLClanReference petition;
    XML_VARIABLE XMLShort clanLevel;
    XML_VARIABLE XMLHometownReference   hometown;

    XML_VARIABLE XMLAttributes attributes;
    XML_VARIABLE Remorts remorts;
    XML_VARIABLE XMLIntegerNoEmpty trust;
    XML_VARIABLE XMLInflectedString russianName;

    XML_VARIABLE XMLStringNoEmpty title;
    XML_VARIABLE XMLStringNoEmpty pretitle;
    XML_VARIABLE XMLStringNoEmpty russianPretitle;
    XML_VARIABLE XMLMultiString description;
    XML_VARIABLE PCSkills skills;
    XML_VARIABLE PCBonuses bonuses;
    XML_VARIABLE XMLInteger security;
    XML_VARIABLE XMLIntegerNoEmpty     questpoints;        
    XML_VARIABLE XMLInteger start_room;
    XML_VARIABLE XMLIntegerNoEmpty        loyalty;          // real ethos - dynamic

    CachedNoun cachedNoun;

public:
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
    XML_VARIABLE XMLInteger                last_death_time;
    XML_VARIABLE XMLIntegerNoEmpty        ghost_time;
    XML_VARIABLE XMLIntegerNoEmpty        PK_time_v;
    XML_VARIABLE XMLIntegerNoEmpty        PK_time_sk;
    XML_VARIABLE XMLIntegerNoEmpty        PK_time_t;
    XML_VARIABLE XMLIntegerNoEmpty        PK_flag;        /* KILLER, SLAIN, VIOLENT, GHOST, THIEF */
    XML_VARIABLE XMLIntegerNoEmpty        death;
    XML_VARIABLE XMLIntegerNoEmpty        anti_killed;
    XML_VARIABLE XMLIntegerNoEmpty        has_killed;
    
    // player-specific parameters
    XML_VARIABLE XMLInteger                perm_hit;
    XML_VARIABLE XMLInteger                perm_mana;
    XML_VARIABLE XMLInteger                perm_move;
    XML_VARIABLE XMLIntegerNoEmpty        practice;
    XML_VARIABLE XMLIntegerNoEmpty        train;

    /** Bonus to skill knowledge through affects. */
    GlobalArray            mod_skills;
    /** Bonus to skill group knowledge through affects. */
    GlobalArray            mod_skill_groups;
    /** Bonus to knowledge of all skills. */
    int                    mod_skill_all;
    /** Bonus to spell/skill level for particular skill. */
    GlobalArray            mod_level_skills;
    /** Bonus to spell/skill level for particular skill group. */
    GlobalArray            mod_level_groups;
    /** General spell/skill level bonus. */
    int                    mod_level_all;
    /** Spell level bonus. */
    int mod_level_spell;

    /** Amount of silver in the bank. */
    XML_VARIABLE XMLIntegerNoEmpty        bank_s;
    /** Amount of gold in the bank. */
    XML_VARIABLE XMLIntegerNoEmpty        bank_g;
    
    NPCharacter *        pet;

    // fields used by skills
    PCharacter *        guarding;
    PCharacter *        guarded_by;
    XML_VARIABLE XMLInteger          shadow;                // INVADER (shadow)
    
    // config
    XML_VARIABLE XMLFlags config;
    bool                      confirm_delete;

    // switch
    NPCharacter                *switchedTo;
};

#endif


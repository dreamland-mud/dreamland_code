/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          character.h  -  description
                             -------------------
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/


#ifndef CHARACTER_H
#define CHARACTER_H

#include <sstream>
#include <stdarg.h>

#include "dlstring.h"
#include "nounholder.h"

#include "core/fenia/wrappertarget.h"

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlshort.h"
#include "xmlinteger.h"
#include "xmlenumeration.h"
#include "xmlflags.h"
#include "xmllonglong.h"
#include "xmlglobalbitvector.h"

#include "pcmemoryinterface.h"
#include "clanreference.h"
#include "profession.h"
#include "race.h"
#include "religion.h"
#include "affectlist.h"

class PCharacter;
class NPCharacter;
class Object;
class SchedulerTask;
class Descriptor;
typedef Pointer<SchedulerTask> SchedulerTaskPointer;
class SpellTarget;
typedef Pointer<SpellTarget> SpellTargetPointer;
class Room;
struct extra_exit_data;
struct exit_data;

enum {
    FMT_NONE     = 0,
    FMT_INVIS    = 1,
    FMT_DOPPEL   = 2,
    FMT_PRETITLE = 4,
};

struct PlayerConfig {
    PlayerConfig( );
    PlayerConfig( const PCharacter * );

    bool holy;
    bool color;
    bool ruskills;
    bool runames;
    bool rucommands;
    bool ruexits;
    bool ruother;
};


/**
 * @author Igor S. Petrenko
 */
class Character : public Grammar::NounHolder,
                  public virtual CharacterMemoryInterface, 
                  public XMLVariableContainer,
                  public WrapperTarget
{                
XML_OBJECT
public:
    typedef ::Pointer<Character> Pointer;

public:
    Character( );
    virtual ~Character( );
    
    // recycle
    virtual void extract( );
    virtual void init( );

    // gate to pc/npc info    
    virtual PCharacter *getPC( ) = 0;
    virtual NPCharacter *getNPC( ) = 0;
    virtual const PCharacter *getPC( ) const = 0;
    virtual const NPCharacter *getNPC( ) const = 0;
    virtual bool is_npc( ) const = 0;
    virtual PCharacter * getPlayer( );
    virtual NPCharacter * getMobile( );
    
    // set-get methods inherited from CharacterMemoryInterface
    virtual const DLString& getName( ) const ;
    virtual void setName( const DLString& ) ;

    virtual short getLevel( ) const ;
    virtual void setLevel( short ) ;

    virtual ReligionReference & getReligion( ) ;
    virtual void setReligion( const ReligionReference & );

    virtual short getSex( ) const ;
    virtual void setSex( short ) ;

    virtual RaceReference &getRace( ) ;
    virtual void setRace( const RaceReference & ) ;

    virtual ClanReference &getClan( ) ;
    virtual void setClan( const ClanReference & ) ;

    virtual ProfessionReference &getProfession( ) ;
    virtual void setProfession( const ProfessionReference & ) ;

    // various set-get methods 
    inline long long getID( ) const;
    inline void setID( long long );
    
    inline bool isDead( ) const;
    inline void setDead( );

    const GlobalBitvector & getWearloc( );
    
    // level
    short getRealLevel( ) const;
    virtual short getModifyLevel( ) const = 0;

    // name and sex formatting
    virtual const char * getNameC( ) const;
    DLString sees( const Character *whomsee, char needcase = '1' ) const;
    DLString seeName( const Character *whomsee, char needcase = '1' ) const;

    // text output (implemented in descriptor)
    void send_to( const char * );
    void send_to( const DLString& );
    void send_to( ostringstream& );
    void vpecho( const char *, va_list );
    void pecho( const DLString &line );
    void pecho( const char *, ... );
    void pecho( int pos, const char *, ... );
    void recho( int, const char *, ... );
    void recho( const char *, ... );
    void recho( Character *, const char *, ... );
    void echo( int pos, int type, Character *vch, const char *f, ... );
    void vecho( int pos, int type, Character *, const char *, va_list );
    void vecho( int pos, int type, Character *, const char *, va_list, bool (needsOutput)(Character *));
    
    // visibility of things 
    bool can_see( const Character *victim ) const;
    bool can_see( const Object *obj ) const;
    bool can_hear( const Object *obj ) const;
    bool can_see( Room *pRoomIndex ) const;
    bool can_see( struct extra_exit_data *peexit ) const;
    bool can_see( struct exit_data *pexit ) const;
    bool can_sense( const Character * ) const;
    
    // object manipulations
    short getWearLevel( Object * );

    // trust and immortality 
    virtual bool isCoder( ) const = 0;
    virtual bool is_immortal( ) const = 0;
    
    // skills
    int getSkill( int );

    // stats
    virtual int getCurrStat( int ) = 0;
   
    // wait and daze state, fight delay time (implemented in fight_core)
    void setWaitViolence( int );
    void setWait( int );
    void setDaze( int );
    void setDazeViolence( int );
    time_t getLastFightDelay( ) const;
    void setLastFightTime( );
    void unsetLastFightTime( );
    time_t getLastFightTime( ) const;
    bool is_adrenalined( ) const;

    // carrying capacity (implemented in anatolia_core)
    int canCarryNumber( ); 
    int canCarryWeight( );
    int getCarryWeight( ) const;

    // misc
    virtual bool is_vampire( ) const = 0;
    virtual bool is_mirror( ) const = 0;
    const Character * getDoppel( const Character *looker = NULL ) const;
    Character * getDoppel( const Character *looker = NULL );
    bool canEnter( Room *room );
    bool isAffected( int ) const;
    void dismount( );
    
    // configuration
    virtual PlayerConfig getConfig( ) const = 0;

protected:
    XML_VARIABLE XMLLongLong ID;
    bool dead;
    XML_VARIABLE XMLString name;
    XML_VARIABLE XMLShort level;
    XML_VARIABLE XMLClanReference clan;
    XML_VARIABLE XMLProfessionReference profession;
    XML_VARIABLE XMLShort sex;
    XML_VARIABLE XMLRaceReference race;
    XML_VARIABLE XMLReligionReference religion;
    time_t                        last_fight_time;

public:
    bool extracted;
    Character*                         reply;
    Character*                         next;
    Character*                         prev;
    Character *                        next_in_room;
    Character *                        master;
    Character *                        leader;
    Character *                        doppel;
    Character *                        fighting;
    Character *                        last_fought;
    Descriptor *                desc;
    AffectList                  affected;
    Object *                        carrying;
    Object *                        on;
    Room *                        in_room;
    Room *                        was_in_room;
    XML_VARIABLE                XMLEnumeration ethos;
    
    int                                timer;
    int                                wait;
    int                                daze;
    
    XML_VARIABLE XMLInteger                hit;
    XML_VARIABLE XMLInteger                max_hit;
    XML_VARIABLE XMLInteger                mana;
    XML_VARIABLE XMLInteger                max_mana;
    XML_VARIABLE XMLInteger                move;
    XML_VARIABLE XMLInteger                max_move;
    XML_VARIABLE XMLIntegerNoEmpty        gold;
    XML_VARIABLE XMLIntegerNoEmpty        silver;
    XML_VARIABLE XMLInteger                exp;
    
    // wizard stuff
    XML_VARIABLE XMLIntegerNoEmpty        invis_level;
    XML_VARIABLE XMLIntegerNoEmpty        incog_level;
    
    // text output
    XML_VARIABLE XMLString                prompt;
    XML_VARIABLE XMLString                batle_prompt;
    char *                prefix;
    XML_VARIABLE XMLInteger                lines;  // for the pager

    XML_VARIABLE XMLFlags                act;
    XML_VARIABLE XMLFlags                comm;   
    XML_VARIABLE XMLFlags                add_comm;
    XML_VARIABLE XMLFlags                imm_flags;
    XML_VARIABLE XMLFlags                res_flags;
    XML_VARIABLE XMLFlags                vuln_flags;
    XML_VARIABLE XMLFlags                affected_by;
    XML_VARIABLE XMLFlags                detection;
    XML_VARIABLE XMLEnumeration        position;
    XML_VARIABLE XMLFlags posFlags;
    XML_VARIABLE XMLGlobalBitvector wearloc;

    int                        carry_weight;
    int                        carry_number;
    XML_VARIABLE XMLIntegerNoEmpty        saving_throw;
    XML_VARIABLE XMLInteger                alignment;
    XML_VARIABLE XMLIntegerNoEmpty        hitroll;
    XML_VARIABLE XMLIntegerNoEmpty        damroll;
    XML_VARIABLE XMLEnumerationArray    armor;
    XML_VARIABLE XMLIntegerNoEmpty   wimpy;
    int                        dam_type;
    int heal_gain;
    int mana_gain;

    // Bonus to wait state beats, in %%.
    int mod_beats;

    // stats 
    XML_VARIABLE XMLEnumerationArray perm_stat;
    XML_VARIABLE XMLEnumerationArray mod_stat;

    // parts stuff 
    XMLFlags   form;
    XMLFlags        parts;
    int        size;
    char*        material;
    
    // hunt data 
    int         endur;
    
    // mount data 
    bool        riding;        
    Character *        mount;

    char        *ambushing;
    
    // traps
    int        death_ground_delay;
    Flags trap;
};

/*
 * inline set-get methods for protected fields
 */
inline long long Character::getID( ) const
{
    return ID.getValue( );
}
inline void Character::setID( long long id )
{
    ID = id;
}
inline bool Character::isDead( ) const
{
    return dead;
}
inline void Character::setDead( )
{
    dead = true;
}


#endif

/* $Id: basicmobilebehavior.h,v 1.1.2.2.6.11 2009/03/16 19:53:39 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef BASICMOBILEBEHAVIOR_H
#define BASICMOBILEBEHAVIOR_H

#include <list>

#include "xmlmap.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmllonglong.h"

#include "mobilebehavior.h"

class Room;
class PCharacter;
class Skill;
class SkillReference;
struct exit_data;

struct MobileMemory : public XMLMapBase<XMLLongLong> {
    MobileMemory( );
    void remember( Character * );
    bool forget( Character * );
    bool memorized( Character * );
    void poll( int );
};

class BasicMobileBehavior : public virtual MobileBehavior {
XML_OBJECT
public:
    typedef ::Pointer<BasicMobileBehavior> Pointer;

    BasicMobileBehavior( );
    virtual ~BasicMobileBehavior( );
   
/*
 * triggers and MobileBehavior interface
 */
    virtual bool area( );
    virtual bool spell( Character *caster, int, bool ); 
    virtual bool kill( Character * );
    virtual bool extractNotify( Character *, bool, bool );
    virtual bool isSaved( ) const;
    virtual bool hasDestiny( );
    virtual int  getOccupation( );
    virtual bool canCancel( Character * );
    virtual bool isAfterCharm( ) const;
    virtual long long getLastCharmTime() const;
    void setLastCharmTime();
    void unsetLastCharmTime();

    void rememberFought(Character *victim);

protected:
    Character * getMaster( Character * );
    int beforeSpell;
    XML_VARIABLE XMLLongLongNoEmpty lastCharmTime;

    static const int AI_CHARM_RECOVER_TIME = 60 * 60;
    static const int AI_ADRENALINE_TIME    = 15 * 60;

/*
 * our last fought victim
 */
protected:
    bool hasLastFought( ) const;
    void setLastFought( Character * );
    PCharacter *getLastFoughtRoom( );
    PCharacter *getLastFoughtWorld( );
    void clearLastFought( );
    bool isLastFought( Character * );
    bool isAdrenalined( ) const;
    XML_VARIABLE XMLStringNoEmpty lastFought;

/*
 * memory of all victims we fought or attacked
 */
    XML_VARIABLE MobileMemory memoryFought;
    XML_VARIABLE MobileMemory memoryAttacked;

/*
 * home point 
 */
    bool backHome( bool );
    bool checkLastFoughtHiding();
    virtual bool isHomesick( );
    void remember( Room * );
    bool findMemoryFoughtRoom(Room *room);
    XML_VARIABLE XMLIntegerNoEmpty homeVnum;

/*
 * experience bonus
 */
public:
    virtual int getExpBonus( Character * );
private:
    struct FindAllEnemiesComplete;
    static void findEnemiesRoom( Room *, int, list<NPCharacter *> &, list<PCharacter *> & );

/*
 * special
 */
public: 
    virtual bool spec( );
protected:    
    virtual bool specFight( );
    virtual bool specAdrenaline( );
    virtual bool specIdle( );

    bool doInvis();    
    bool doCallHelp( );
    bool doHeal( );
    bool doPickWeapon( );
    bool doQuaff( );
    bool doScavenge( );
    bool doWander( );
    bool doWimpy( );
    bool mustFlee( );

/*
 * aggression
 */
public: 
    virtual bool aggress( );
protected:    
    bool canAggress( );
    bool canAggress( Character * );
    virtual bool canAggressNormal( Character * );
    virtual bool aggressNormal( );
    bool canAggressLastFought( Character * );
    virtual bool aggressLastFought( );
    virtual bool aggressMemorized( );
    bool aggressRanged( );
private:
    Character * findRangeVictim( int, int &, int & );

/*
 * tracking
 */
public:
    virtual void shot( Character *, int );
    virtual bool track( );
    virtual void flee( );
protected:    
    bool move( int, struct exit_data *, Character * );
    bool canTrack( );
    bool canTrackLastFought( Character * );
    virtual bool trackLastFought( Character * );
    XML_VARIABLE XMLBooleanNoFalse lostTrack;

/*
 * fighting
 */
public:
    virtual void fight( Character * );
protected:    
    virtual void attack( Character * );;
    virtual void attackSmart( Character * );
    void attackDumb( Character * );

/*
 * auto-assist
 */
public:
    virtual bool assist( Character *, Character * );
protected:
    bool assistOffense( Character *, Character * );
    bool canAssistOffense( Character *, Character * );
    bool assistMaster( Character *, Character * );
    bool assistGroup( Character *, Character * );
    bool assistGroupDistance( Character *, Character * );
    bool assistGroupHealing( Character * );
    bool canAssistGroup( Character *, Character * );
    bool assistSpell( NPCharacter *, SkillReference &, Character * );
private:
    struct FindAssistersComplete;
    int findRetreatDoor( );
    Character * findAssistVictim( Character * );
    
/*
 * common caster brain
 */
protected:
    struct SpellChance {
        int gsn;
        int chance;
    };
    struct SpellChanceTable {
        SpellChanceTable( const SpellChance *, NPCharacter *, Character * );

        bool canCastSpell( int );
        bool castSpell( int = 0xFFFF );
        int  findSpell( );
        int  findRangedSpell( int );

        const SpellChance * spellTable;
        NPCharacter * ch;
        Character   * victim;
    };

    bool specFightCaster( );
    bool aggressCaster( );
    bool canAggressDistanceCaster( Character * );
    int casterSnRange( Character *, int );
    bool trackCaster( Character * );
private:
    Character * findCastVictim( );

/*
 * cleric brain
 */
protected:
    static SpellChance clericSnHealing [];
    static SpellChance clericSnAttack [];
    static SpellChance clericSnPanicAttack [];
    static SpellChance clericSnPassiveDefence [];
    static SpellChance clericSnCurative [];
    static SpellChance clericSnRange [];
    bool specFightCleric( );
    bool healCleric( Character * );

/*
 * mage brain
 */
    static SpellChance mageSnAttack [];
    static SpellChance mageSnPanicAttack [];
    static SpellChance mageSnPassiveDefence [];
    static SpellChance mageSnRange [];
    bool specFightMage( );

/*
 * vampire brain
 */
    struct VampVictims;
    struct TouchVictims;
    struct BiteVictims;
    struct SuckVictims;
    struct DispelVictims;
    struct BlindVictims;
    struct KillVictims;
    bool specFightVampire( );
    bool aggressVampire( );
    virtual bool canAggressVampire( Character * );

/*
 * necromancer and undead brain
 */
    static SpellChance necroSnAttack [];
    static SpellChance necroSnPanicAttack [];
    static SpellChance necroSnPassiveDefence[];
    static SpellChance necroSnHealing [];
    static SpellChance necroSnRange [];
    bool specFightNecro( );
    bool healNecro( Character * );

/*
 * ranger brain
 */
    bool canAggressDistanceRanger( );
    bool aggressRanger( );
    bool healRanger( Character * );

/*
 * item usage
 */
    bool useItemWithSpell( int, Character * );
};

class BasicMobileDestiny : public BasicMobileBehavior {
XML_OBJECT
public:
    typedef ::Pointer<BasicMobileDestiny> Pointer;

    virtual bool isSaved( ) const;
    virtual bool hasDestiny( );
};

class SavedCreature : public BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<SavedCreature> Pointer;
    
    virtual void save( );
    virtual bool extract( bool );
    virtual void stopfol( Character * );
    virtual bool hasSpecialName() const { return true; }

protected:
    XML_VARIABLE XMLBoolean saved;
};


#endif

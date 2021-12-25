/* $Id: hunter.h,v 1.1.6.2.6.4 2007/09/15 09:24:10 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef HUNTER_H 
#define HUNTER_H 

#include "clanmobiles.h"

#include "xmlmap.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"

#include "skill.h"

class Affect;
class Room;

class ClanGuardHunter : public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardHunter> Pointer;
    
protected:        
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual int getCast( Character * );

        void createEquipment( PCharacter * );
};

class ClanHealerHunter: public ClanHealer {
XML_OBJECT
public:
        typedef ::Pointer<ClanHealerHunter> Pointer;
    
        void speech( Character *wch, const char *speech );
        void tell( Character *wch, const char *speech );
};

class ClanAreaHunter: public ClanArea {
XML_OBJECT
friend class ClanHealerHunter;
friend class ClanGuardHunter;
public:
        typedef ::Pointer<ClanAreaHunter> Pointer;
        typedef XMLMapBase<XMLInteger> Weapons;

protected:
        XML_VARIABLE Weapons weapons;
        XML_VARIABLE XMLInteger armorVnum;

        Object * createEquipment( PCharacter * );
        Object * createArmor( PCharacter * );
        Object * createWeapon( PCharacter *, int );
        int  vnumByString( const DLString& );
};

class HunterEquip : public ClanObject {
XML_OBJECT
friend class ClanAreaHunter;
public:
        typedef ::Pointer<HunterEquip> Pointer;
        
        HunterEquip( );

        virtual void get( Character * );
        virtual bool canEquip( Character * );
        virtual bool mayFloat( ); 

protected:
        void config( PCharacter * );
};

class HunterWeapon : public HunterEquip {
XML_OBJECT
public:
        typedef ::Pointer<HunterWeapon> Pointer;
        
        virtual void wear( Character * );
        virtual void fight( Character * );
        
protected:
        void fight_axe( Character * );
        void fight_mace( Character * );
        void fight_sword( Character * );
};

class HunterArmor : public HunterEquip {
XML_OBJECT
public:
        typedef ::Pointer<HunterArmor> Pointer;
        
        virtual void wear( Character * );
        virtual bool canLock( Character * );
        virtual void delete_( Character * ); 

protected:
        void addAffect( Character *, Affect * );
};

class HunterTrapObject : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<HunterTrapObject> Pointer;
    
    HunterTrapObject( );

    virtual bool visible( const Character * );

protected:
    virtual bool checkPrevent( Character * );
    virtual bool checkRoom( Room * );
    virtual bool checkTrapConditions( Character *, Skill & );

    void log( Character *, const char * );

    XML_VARIABLE XMLString ownerName;
    XML_VARIABLE XMLInteger ownerLevel;
    XML_VARIABLE XMLBoolean activated;
    XML_VARIABLE XMLString activeDescription;
};

class HunterBeaconTrap : public HunterTrapObject {
XML_OBJECT
public:
    typedef ::Pointer<HunterBeaconTrap> Pointer;

    virtual bool use( Character *, const char * );
    virtual void greet( Character * );
    virtual bool hasTrigger( const DLString &  );

protected:
    XML_VARIABLE XMLString victimName;
    XML_VARIABLE XMLInteger quality;
    XML_VARIABLE XMLInteger charges;
};

class HunterSnareTrap : public HunterTrapObject {
XML_OBJECT
public:
    typedef ::Pointer<HunterSnareTrap> Pointer;

    virtual bool use( Character *, const char * );
    virtual void greet( Character * );
    virtual void fight( Character * );
    virtual void entry( );
    virtual bool hasTrigger( const DLString &  );
    
    int getQuality( ) const;

protected:
    virtual bool checkRoom( Room * );

    XML_VARIABLE XMLInteger quality;
};

class HunterShovel : public HunterTrapObject {
XML_OBJECT
public:
    typedef ::Pointer<HunterShovel> Pointer;

    virtual bool use( Character *, const char * );
    virtual bool hasTrigger( const DLString &  );

protected:
    virtual bool checkRoom( Room * );
};

class HunterPitSteaks : public HunterTrapObject {
XML_OBJECT
public:    
    typedef ::Pointer<HunterPitSteaks> Pointer;

    virtual bool use( Character *, const char * );
    virtual bool hasTrigger( const DLString &  );
};

class HunterPitTrap : public HunterTrapObject {
XML_OBJECT
public:    
    typedef ::Pointer<HunterPitTrap> Pointer;

    virtual void greet( Character * );
    virtual bool area( );
    
    int getQuality( ) const;
    void setDepth( int );
    int getDepth( ) const;
    int getSize( ) const;
    void setReady( Character * );
    void unsetReady( );
    void setDescription( );
    Object * getSteaks( );
    bool isOwner( Character * ) const;
    bool isFresh( ) const;
    void setOwner( Character * );

protected:
    XML_VARIABLE XMLInteger depth;
    XML_VARIABLE XMLInteger quality;
};


#endif

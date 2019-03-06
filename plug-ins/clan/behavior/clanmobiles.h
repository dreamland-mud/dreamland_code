/* $Id$
 *
 * ruffina, 2004
 */
#ifndef CLANMOBILES_H
#define CLANMOBILES_H

#include "basicmobilebehavior.h"
#include "healer.h"
#include "clanobjects.h"
#include "summoncreaturespell.h"
#include "savedcreature.h"

class PCharacter;
class Object;

class ClanMobile: public virtual MobileBehavior {
public:
        typedef ::Pointer<ClanMobile> Pointer;
        
        virtual void setChar( NPCharacter * );
        virtual void unsetChar( );

protected:
        ClanArea::Pointer getClanArea( );

        XML_VARIABLE XMLClanReference clan;
};

class ClanHealer : public ClanMobile, public Healer {
XML_OBJECT
public:
        typedef ::Pointer<ClanHealer> Pointer;
    
        ClanHealer( );

        virtual int  getOccupation( );
protected:
        virtual bool specIdle( );
        virtual bool canServeClient( Character * );
};

class ClanGuard : public ClanMobile, public BasicMobileDestiny {
public:
        typedef ::Pointer<ClanGuard> Pointer;
    
        ClanGuard( );
        
        virtual void greet( Character * );
        virtual void speech( Character *, const char * );
        virtual bool aggress( );
        virtual int  getOccupation( );

protected:        
        virtual bool checkPush( PCharacter * );
        virtual bool checkGhost( PCharacter * ); 
        
        virtual void actInvited( PCharacter *, Object * );
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
        virtual void actIntruder( PCharacter * );
        virtual void actGhost( PCharacter * );
        virtual void actGiveInvitation( PCharacter *, Object * );
        
        virtual bool specFight( );
        virtual bool specAdrenaline( );
        virtual bool spec_cast( Character * );
        virtual int getCast( Character * );

        Character * getVictim( );
        void doAttack( PCharacter * );
        void doPetitionOutsider( PCharacter * );
};

class ClanSummonedCreature : public SummonedCreature, 
                             public SavedCreature,
                             public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<ClanSummonedCreature> Pointer;
        
        virtual ~ClanSummonedCreature( );
};

#endif


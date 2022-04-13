/* $Id: ruler.h,v 1.1.6.5.6.2 2007/06/26 07:10:43 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef RULER_H
#define RULER_H

#include "clanmobiles.h"
#include "wanderer.h"

class ClanGuardRulerPre : public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardRulerPre> Pointer;
    
        virtual void greet( Character * );
        virtual void speech( Character *, const char * );

protected:        
        virtual bool specFight( );
        
        virtual void actInvited( PCharacter *, Object * );
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
};

class ClanGuardRuler : public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardRuler> Pointer;
    
protected:        
        virtual int getCast( Character * );
};

class ClanGuardRulerJailer : public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardRulerJailer> Pointer;
    
        virtual void greet( Character * );
        virtual void speech( Character *, const char * );

protected:        
        virtual bool specFight( );
        virtual void actPush( PCharacter * );
        virtual void actIntruder( PCharacter * );
};

class RulerSpecialGuard : public ClanSummonedCreature {
XML_OBJECT
public:
        typedef ::Pointer<RulerSpecialGuard> Pointer;
    
protected:    
        virtual bool specFight( );
};


class Stalker : public Wanderer, public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<Stalker> Pointer;
    
        Stalker( );
        
        inline void setVictimName( const DLString & );
        
        virtual void entry( );
        virtual void greet( Character * );
        virtual bool death( Character * );
        virtual bool kill( Character * );

protected:
        virtual bool specAdrenaline( );
        virtual bool specIdle( );
        virtual bool canEnter( Room *const );

private:
        Character *findVictimHere( );
        Character *findVictimWorld( );
        bool ourVictim( Character * );
        void attackVictim( Character * );
        void clantalk( const char * );

        XML_VARIABLE XMLString victimName;
};

inline void Stalker::setVictimName( const DLString &name )
{
    victimName = name;
}

#endif

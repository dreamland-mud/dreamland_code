/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __BATTLERAGER_H__
#define __BATTLERAGER_H__

#include "xmlstring.h"
#include "xmlattribute.h"
#include "objectbehavior.h"
#include "personalquestreward.h"
#include "clanmobiles.h"

class Object;

class BattleragerPoncho : public virtual ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<BattleragerPoncho> Pointer;

    virtual void wear( Character * );
    virtual void remove( Character * );
};

class PersonalBattleragerPoncho : public BattleragerPoncho, public PersonalQuestReward {
XML_OBJECT
public:
    typedef ::Pointer<PersonalBattleragerPoncho> Pointer;
    
    virtual ~PersonalBattleragerPoncho( );
};


class ClanGuardBattlerager: public ClanGuard {
XML_OBJECT
public:
        typedef ::Pointer<ClanGuardBattlerager> Pointer;
    
protected:        
        virtual bool specFight( );
        virtual void actPush( PCharacter * );
        virtual void actGreet( PCharacter * );
};

class ClanHealerBattlerager: public ClanMobile, public BasicMobileDestiny {
XML_OBJECT
public:
        typedef ::Pointer<ClanHealerBattlerager> Pointer;
        
        ClanHealerBattlerager( );

        virtual void speech( Character *wch, const char *speech );

protected:
        XML_VARIABLE XMLBoolean healPets;
};

#endif

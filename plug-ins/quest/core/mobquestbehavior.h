/* $Id: mobquestbehavior.h,v 1.1.4.4.6.3 2008/03/26 14:51:04 rufina Exp $
 *
 * ruffina, 2003
 */

#ifndef MOBQUESTBEHAVIOR_H
#define MOBQUESTBEHAVIOR_H

#include "questentity.h"
#include "basicmobilebehavior.h"
#include "xmlflags.h"

class MobQuestBehavior : public BasicMobileDestiny, 
                         public virtual QuestEntity 
{
XML_OBJECT
public:
    MobQuestBehavior( );

    virtual void setChar( NPCharacter * );
    virtual void unsetChar( );

protected:
    virtual bool isHomesick( );

    PCharacter * getHeroRoom( );

    XML_VARIABLE XMLFlags imm_flags;
    XML_VARIABLE XMLFlags act_flags;
};

class MandatoryMobile : public virtual MobQuestBehavior {
public:
    virtual bool extract( bool );
};

class ConfiguredMobile : public virtual MobQuestBehavior {
public:
    virtual void setChar( NPCharacter * );

protected:
    virtual void config( PCharacter * ) = 0;
};

class TalkativeClient : public virtual MobQuestBehavior {
public:
    virtual void greet( Character * );
    virtual void entry( );

protected:
    virtual bool specIdle( );
    virtual void talkToHero( PCharacter * ) = 0;
};

class PeacefulClient : public virtual MobQuestBehavior {
protected:
    virtual bool canAggressNormal( Character * );
    virtual bool canAggressVampire( Character * );
};

class HuntedVictim : public virtual MobQuestBehavior {
public:
    virtual bool death( Character * );

protected:
    virtual void deadFromHunter( PCMemoryInterface * );
    virtual void deadFromSuicide( PCMemoryInterface * );
    virtual void deadFromOther( PCMemoryInterface *, Character * );
    virtual void deadFromGroupMember( PCMemoryInterface *, Character * );
};

class ProtectedClient : public virtual MobQuestBehavior {
public:
    virtual bool death( Character * );

protected:
    virtual void deadAction( QuestPointer, PCMemoryInterface *, Character * );
    virtual void deadFromIdiot( PCMemoryInterface * );
    virtual void deadFromSuicide( PCMemoryInterface * );
    virtual void deadFromKill( PCMemoryInterface *, Character * );
};

template <typename Q>
class DedicatedMobile : public virtual MobQuestBehavior {
protected:
    inline bool getQuest( )
    {
        return quest || (quest = getMyQuest<Q>( ));
    }

    ::Pointer<Q> quest;
};

class GreedyClient : public virtual MobQuestBehavior {
public:
    virtual void give( Character *, Object * );
    
protected:
    virtual bool givenCheck( PCharacter *, Object * ) = 0;
    virtual void givenGood( PCharacter *, Object * ) = 0;
    virtual void givenBad( PCharacter *, Object * ) = 0;
};

#endif



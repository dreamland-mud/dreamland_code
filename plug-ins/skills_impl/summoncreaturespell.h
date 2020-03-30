/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __SUMMONCREATURESPELL_H__
#define __SUMMONCREATURESPELL_H__

#include "xmlstring.h"
#include "xmllonglong.h"
#include "xmlinteger.h"
#include "spelltemplate.h"
#include "mobilebehavior.h"

class SummonCreatureSpell : public virtual DefaultSpell {
XML_OBJECT
public:
    typedef ::Pointer<SummonCreatureSpell> Pointer;

    virtual void run( Character *, Character *, int, int );
    virtual void run( Character *, char *, int, int );

protected:
    int countMobiles( Character * ) const;
    NPCharacter * createMobileAux( Character *, int, int, int, int, int, int ) const;
    virtual NPCharacter * createMobile( Character *, int ) const = 0;
    virtual bool canSummonHere( Character * ) const = 0;

    XML_VARIABLE XMLInteger maxMobCount;
    XML_VARIABLE XMLInteger castMobCount;
    XML_VARIABLE XMLInteger postaffectDuration;
    XML_VARIABLE XMLInteger mobVnum;
    XML_VARIABLE XMLString msgStillAffected;
    XML_VARIABLE XMLString msgTooManyMobiles;
    XML_VARIABLE XMLString msgCreateAttemptSelf, msgCreateAttemptRoom, msgCreateAttemptArea;
    XML_VARIABLE XMLString msgCreateSelf, msgCreateRoom, msgCreateSelfOne, msgCreateRoomOne;
    XML_VARIABLE XMLString msgNotRecognizeRoom, msgNotRecognizeSelf;
    XML_VARIABLE XMLString msgReattachAttemptRoom, msgReattachAttemptSelf;
    XML_VARIABLE XMLString msgReattachRoom, msgReattachSelf;
};

// MOC_SKIP_BEGIN
template <const char *&tn>
class SpellTemplate<tn, SummonCreatureSpell> : 
        public SummonCreatureSpell, 
        public ClassSelfRegistratorPlugin<tn> {
public:
    virtual void run( Character *ch, Character *victim, int sn, int level ) 
    {
        SummonCreatureSpell::run( ch, victim, sn, level );
    }
    virtual void run( Character *ch, char *arg, int sn, int level ) 
    {
        SummonCreatureSpell::run( ch, arg, sn, level );
    }

    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
protected:    
    virtual NPCharacter * createMobile( Character *, int ) const 
    { 
        return 0; 
    }
    virtual bool canSummonHere( Character * ) const 
    {
        return true;
    }
};
// MOC_SKIP_END

class SummonedCreature : public virtual MobileBehavior {
XML_OBJECT
public:
    typedef ::Pointer<SummonedCreature> Pointer;
    
    virtual void conjure( );

    XML_VARIABLE XMLLongLong creatorID;
    XML_VARIABLE XMLString creatorName;
};

#endif
    

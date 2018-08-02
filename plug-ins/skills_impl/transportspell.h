/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __TRANSPORTSPELL_H__
#define __TRANSPORTSPELL_H__

#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "spelltemplate.h"
#include "jumpmovement.h"

/*
 * gate
 */
class GateSpell : public virtual DefaultSpell {
XML_OBJECT
public:
    typedef ::Pointer<GateSpell> Pointer;
    
    virtual void run( Character *, Character *, int, int );

    XML_VARIABLE XMLBoolean gateShadow, gateViolent;
    XML_VARIABLE XMLBoolean takePet, takeGroup;
    XML_VARIABLE XMLString  msgRoomLeave, msgRoomEnter;
    XML_VARIABLE XMLString  msgSelfLeave, msgSelfEnter;
    XML_VARIABLE XMLString  msgGroupLeave, msgGroupEnter;
    XML_VARIABLE XMLInteger levelDiff;
};

// MOC_SKIP_BEGIN
template <const char *&tn>
class SpellTemplate<tn, GateSpell> : public ClassSelfRegistratorPlugin<tn>, public GateSpell {
public:
    virtual void run( Character *ch, Character *victim, int sn, int level ) 
    {
	GateSpell::run( ch, victim, sn, level );
    }
    virtual const DLString &getType( ) const {
        return ClassSelfRegistratorPlugin<tn>::getType( );
    }
};
// MOC_SKIP_END

class GateMovement : public JumpMovement {
public:
    GateMovement( Character *ch, Character *victim, GateSpell::Pointer spell, Character *actor, int level );
    GateMovement( Character *ch, Character *victim );
    virtual ~GateMovement( );

    virtual bool canMove( Character * );
protected:
    bool checkLevel( );
    bool checkCaster( Character * );
    bool checkCasterRoom( );
    bool checkVictim( );
    bool checkVictimRoom( );

    virtual bool tryMove( Character * );
    bool applySavesSpell( );
    bool applyViolent( Character * );

    virtual bool moveAtomic( );
    virtual void moveFollowers( Character * );
    virtual void msgOnMove( Character *, bool );
    virtual void msgEcho( Character *, Character *, const char * );
    
    Character *victim;
    GateSpell::Pointer spell;
    int level;
};

/*
 * summon
 */
class SummonSpell : public virtual DefaultSpell {
XML_OBJECT
public:
    typedef ::Pointer<SummonSpell> Pointer;
    
    virtual void run( Character *, Character *, int, int );
};

class SummonMovement : public JumpMovement {
public:
    SummonMovement( Character *ch, Character *caster, SummonSpell::Pointer, int level );
    virtual ~SummonMovement( );

protected:
    virtual bool canMove( Character * );
    bool checkCaster( );
    bool checkVictim( );
    
    virtual bool tryMove( Character * );
    bool applySpellbane( );
    bool applySavesSpell( );
    bool applyLazy( );

    virtual bool moveAtomic( );
    virtual void msgOnMove( Character *, bool );
    virtual void msgEcho( Character *, Character *, const char * );

    Character *caster;
    SummonSpell::Pointer spell;
    int level;
};

#endif


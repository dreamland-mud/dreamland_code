/* $Id: defaultspell.h,v 1.1.2.2.18.8 2008/07/04 12:05:08 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef        __DEFAULTSPELL_H__
#define        __DEFAULTSPELL_H__

#include "spell.h"
#include "skill.h"

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlinteger.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlboolean.h"

enum {
    TARGET_ERR_SUMMON_WHO = 1,
    TARGET_ERR_NOT_ON_OTHERS,
    TARGET_ERR_NO_TARGET_NEEDED,
    TARGET_ERR_CAST_ON_WHOM,
    TARGET_ERR_CAST_ON_WHAT,
    TARGET_ERR_CHAR_NOT_FOUND,
    TARGET_ERR_OBJ_NOT_FOUND,
    TARGET_ERR_TOO_FAR
};

/**
 * Base implementation for all types of spells. Individual spells mostly override one of the
 * 'run' methods.
 */
class DefaultSpell : public Spell, public XMLVariableContainer
{
XML_OBJECT
public:
    typedef ::Pointer<DefaultSpell> Pointer;
    
    DefaultSpell( );

    virtual void setSkill( SkillPointer );
    virtual void unsetSkill( );
    virtual SkillPointer getSkill( ) const;

    virtual void run( Character *, SpellTargetPointer, int );             
    virtual void run( Character *, Character *, int, int ) { }
    virtual void run( Character *, Object *, int, int ) { }
    virtual void run( Character *, char *, int, int ) { }
    virtual void run( Character *, Room *, int, int ) { }

    virtual int getManaCost( Character * );
    virtual int getMaxRange( Character * ) const;                
    virtual bool spellbane( Character *, Character * ) const; 
    virtual void utter( Character * );
    virtual int getSpellLevel( Character *, int );

    virtual SpellTargetPointer locateTargets( Character *, const DLString &, std::ostringstream & );
    virtual SpellTargetPointer locateTargetObject( Character *, const DLString &, std::ostringstream & );

    virtual int getBeats( ) const;
    virtual int getMana( ) const;
    virtual int getTarget( ) const;
    virtual int getSpellType( ) const;
    virtual int getPosition() const;
    virtual bool isCasted( ) const;
    virtual bool isPrayer( Character * ) const;
    virtual bool checkPosition( Character * ) const;

protected:
    Character * getCharSpell( Character *, const DLString &, int *, int *, ostringstream &errbuf );
    
    void baneMessage( Character *ch, Character *vch ) const;
    void baneDamage( Character *ch, Character *vch, int dam ) const;
    void baneAround( Character *ch, int failChance, int dam ) const;
    void baneForAssist( Character *ch, Character *vch ) const;
    bool baneAction( Character *ch, Character *bch, int failChance, int dam ) const;

    XML_VARIABLE XMLFlags   target;
    XML_VARIABLE XMLEnumeration   position;
    XML_VARIABLE XMLEnumeration   type;
    XML_VARIABLE XMLBooleanNoTrue casted;
    XML_VARIABLE XMLBooleanNoTrue ranged;

    SkillPointer skill;
};

/**
 * Describes a typical spell from attack or combat group that checks saves roll, inflicts damage
 * and shows some messages.
 */
class AnatoliaCombatSpell : public virtual DefaultSpell {
XML_OBJECT
public:
    typedef ::Pointer<AnatoliaCombatSpell> Pointer;
    
    AnatoliaCombatSpell();

    virtual void run( Character *, Character *, int, int );

    XML_VARIABLE XMLEnumeration damtype; // damage type from damage_table
    XML_VARIABLE XMLFlags damflags; // additional flags other than DAMF_SPELL
    XML_VARIABLE XMLInteger dice, diceBonus; // damage calculation: <level> d <dice> + <diceBonus>
    XML_VARIABLE XMLStringNoEmpty msgNotVict, msgVict, msgChar; // optional messages 
    XML_VARIABLE XMLIntegerNoEmpty waitMin, waitMax; // optional range of waitstate on victim
    XML_VARIABLE XMLBooleanNoTrue savesCheck; // whether to check for saves and reduce damage, true by default
};

#endif

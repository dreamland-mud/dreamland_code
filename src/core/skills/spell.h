/* $Id$
 *
 * ruffina, 2004
 */
#ifndef        __SPELL_H__
#define        __SPELL_H__

#include <sstream>

#include "xmlpolymorphvariable.h"
#include "dlstring.h"
#include "skillaction.h"

class Character;
class Object;
class Room;
class SpellTarget;
typedef ::Pointer<SpellTarget> SpellTargetPointer;

class Spell: public virtual SkillAction, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<Spell> Pointer;
    
    virtual ~Spell( );

    virtual void run( Character *, SpellTargetPointer, int ) = 0;
    virtual void run( Character *, Character *, int, int ) = 0;
    virtual void run( Character *, Object *, int, int ) = 0;
    virtual void run( Character *, char *, int, int ) = 0;
    virtual void run( Character *, Room *, int, int ) = 0;

    virtual int getManaCost( Character * ) = 0;
    virtual int getMaxRange( Character * ) const = 0;
    virtual bool spellbane( Character *, Character * ) const = 0;
    virtual void utter( Character * ) = 0;
    virtual int getSpellLevel( Character *, int ) = 0;

    virtual int getBeats( ) const = 0;
    virtual int getMana( ) const = 0;
    virtual int getTarget( ) const = 0;
    virtual int getSpellType( ) const = 0;
    virtual int getPosition() const = 0;
    virtual bool isCasted( ) const = 0;
    virtual bool isPrayer( Character * ) const = 0;
    virtual bool checkPosition( Character * ) const = 0;

    virtual SpellTargetPointer locateTargets( Character *, const DLString &, std::ostringstream & ) = 0;
    virtual SpellTargetPointer locateTargetObject( Character *, const DLString &, std::ostringstream & ) = 0;
};

#endif

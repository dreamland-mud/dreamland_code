/* $Id$
 *
 * ruffina, 2004
 */
#ifndef        SKILLCOMMAND_H
#define        SKILLCOMMAND_H

#include "lang.h"
#include "xmlpolymorphvariable.h"
#include "skillaction.h"
#include "wrappertarget.h"

class Character;

class SkillCommand: public virtual SkillAction, public virtual WrapperTarget, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<SkillCommand> Pointer;
    
    virtual ~SkillCommand( );

    virtual long long getID() const = 0;
    
    virtual const DLString & getName( ) const;
    virtual const DLString & getRussianName( ) const;
    virtual const DLString& getNameFor( Character * ) const;
    /** Language-only variant, for per-language help titles (no Character). */
    virtual const DLString& getNameFor( lang_t lang ) const;
    virtual bool apply( Character *ch, Character *victim = 0, int level = 0 ) { return false; }
};

#endif

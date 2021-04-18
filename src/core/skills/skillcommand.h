/* $Id$
 *
 * ruffina, 2004
 */
#ifndef        SKILLCOMMAND_H
#define        SKILLCOMMAND_H

#include "xmlpolymorphvariable.h"
#include "skillaction.h"
#include "wrappertarget.h"

class Character;

class SkillCommand: public virtual SkillAction, public WrapperTarget, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<SkillCommand> Pointer;
    
    virtual ~SkillCommand( );

    virtual long long getID() const = 0;
    
    virtual const DLString & getName( ) const;
    virtual const DLString & getRussianName( ) const;
    virtual const DLString& getNameFor( Character * ) const;
    virtual bool run( Character *, Character * ) { return false; }
    virtual bool run( Character * ) { return false; }
    virtual bool run( Character *, int ) { return false; }
};

#endif

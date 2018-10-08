/* $Id$
 *
 * ruffina, 2004
 */
#ifndef        SKILLCOMMAND_H
#define        SKILLCOMMAND_H

#include "xmlpolymorphvariable.h"
#include "skillaction.h"

class Character;

class SkillCommand: public virtual SkillAction, public virtual XMLPolymorphVariable {
public:
    typedef ::Pointer<SkillCommand> Pointer;
    
    virtual ~SkillCommand( );

    virtual const DLString & getName( ) const;
    virtual const DLString & getRussianName( ) const;
    virtual bool run( Character *, Character * ) { return false; }
    virtual bool run( Character * ) { return false; }
    virtual bool run( Character *, int ) { return false; }
    virtual void run( Character *, Character *, Character *& ) { }
    virtual void run( Character *, Character *, int & ) { } 
};

#endif

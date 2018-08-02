/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTSKILLCOMMAND_H
#define DEFAULTSKILLCOMMAND_H

#include "skillcommand.h"
#include "defaultcommand.h"

class DefaultSkillCommand : public DefaultCommand, public SkillCommand {
XML_OBJECT
public:
    typedef ::Pointer<DefaultSkillCommand> Pointer;
    
    DefaultSkillCommand( );

    virtual void setSkill( SkillPointer );
    virtual void unsetSkill( );
    virtual SkillPointer getSkill( ) const;

    virtual const DLString & getName( ) const;
    virtual const DLString & getRussianName( ) const;
    virtual void run( Character *, const DLString & );
    virtual void run( Character *, char * );
    virtual bool run( Character *, Character * );
    virtual bool run( Character * );
    virtual bool run( Character *, int );
    virtual void run( Character *, Character *, Character *& );
    virtual void run( Character *, Character *, int & );
protected:
    SkillPointer skill;
    XML_VARIABLE XMLBooleanNoTrue interp; 
};

#endif

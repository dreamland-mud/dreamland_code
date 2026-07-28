/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEFAULTSKILLCOMMAND_H
#define DEFAULTSKILLCOMMAND_H

#include "skillcommand.h"
#include "command.h"

class Object;
struct CommandTarget {
    CommandTarget();

    Character *vict;
    Object *obj;
    DLString argAll;
    DLString argOne, argTwo;
    int door;
    DLString extraExit;
    DLString doorOrExtraExit;
};

class DefaultSkillCommand : public Command, public SkillCommand {
XML_OBJECT
public:
    typedef ::Pointer<DefaultSkillCommand> Pointer;
    
    DefaultSkillCommand( );

    virtual long long getID() const;
        
    virtual bool saveCommand() const;

    virtual void setSkill( SkillPointer );
    virtual void unsetSkill( );
    virtual SkillPointer getSkill( ) const;

    virtual const DLString & getName( ) const;
    virtual const DLString & getRussianName( ) const;
    /** Both bases now declare a language-only getNameFor, so this class has to
     *  say which one wins. Command's is the right answer: it holds the full
     *  multilingual name, while SkillCommand's only knows EN and RU. */
    virtual const DLString & getNameFor( lang_t lang ) const;
    virtual void run( Character *, const DLString & );
    virtual void run( Character *, char * );
    virtual bool applyLegacy(Character * ch, Character *victim, int level);
    virtual bool apply( Character *ch, Character *victim = 0, int level = 0 );
    virtual bool visible( Character * ) const;

    XML_VARIABLE XMLEnumeration argtype;

protected:
    SkillPointer skill;

    bool parseArguments(Character *actor, const DLString &constArgs, CommandTarget &target, ostringstream &errbuf);
};

#endif

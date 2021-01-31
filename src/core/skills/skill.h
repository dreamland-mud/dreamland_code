/* $Id: skill.h,v 1.1.2.8.6.10 2010-09-05 13:57:11 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __SKILL_H__
#define __SKILL_H__

#include <ostream>

#include "globalregistryelement.h"
#include "globalbitvector.h"

class Character;
class PCharacter;
class NPCharacter;
class CharacterMemoryInterface;
class Spell;
class AffectHandler;
class SkillGroupReference;
class SkillCommand;
class SkillEventHandler;
class HelpArticle;
class RussianString;

typedef ::Pointer<Spell> SpellPointer;
typedef ::Pointer<SkillCommand> SkillCommandPointer;
typedef ::Pointer<AffectHandler> AffectHandlerPointer;
typedef ::Pointer<SkillEventHandler> SkillEventHandlerPointer;
typedef ::Pointer<HelpArticle> HelpArticlePointer;

class Skill : public GlobalRegistryElement {
public:
    typedef ::Pointer<Skill> Pointer;
    
    Skill( );
    Skill( const DLString & );
    virtual ~Skill( );

    virtual const DLString& getName( ) const;
    virtual bool isValid( ) const;
    
    const DLString& getNameFor( Character * ) const;
    virtual const DLString &getRussianName( ) const;
    virtual GlobalBitvector & getGroups();
    bool hasGroup(unsigned int group);
    virtual SpellPointer getSpell( ) const;
    virtual AffectHandlerPointer getAffect( ) const;
    virtual SkillCommandPointer getCommand( ) const;
    virtual HelpArticlePointer getSkillHelp() const;
    virtual SkillEventHandlerPointer getEventHandler( ) const;
    virtual int getBeats( ) const;
    virtual int getMana( ) const;
    virtual const RussianString &getDammsg( ) const;
    virtual bool isPassive() const;

    virtual bool visible( CharacterMemoryInterface * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool message = true ) const; 
    virtual int getLevel( Character * ) const;
    virtual int getLearned( Character * ) const;
    virtual int getEffective( Character * ) const;
    virtual int getAdept( PCharacter * ) const;
    virtual int getMaximum( Character * ) const;
    virtual void improve( Character *, bool, Character *victim = NULL, int dam_type = -1, int dam_flags = 0 ) const;
    
    virtual bool canPractice( PCharacter *, std::ostream & ) const;
    virtual bool canTeach( NPCharacter *, PCharacter *, bool verbose = true );
    virtual void practice( PCharacter * ) const;
    
    virtual void show( PCharacter *, std::ostream & ) const;

    virtual const DLString& getCategory( ) const;

    static GlobalBitvector zeroGroups;
protected:
    DLString name;
};

#endif

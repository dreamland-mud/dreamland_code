/* $Id: basicskill.h,v 1.1.2.2.6.11 2010-09-05 13:57:11 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __BASICSKILL_H__
#define __BASICSKILL_H__

#include <map>
#include "xmlpointer.h"
#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmltableelement.h"
#include "xmlrussianstring.h"
#include "xmlflags.h"
#include "xmlmultistring.h"

#include "skill.h"
#include "affecthandler.h"
#include "spell.h"
#include "skillcommand.h"
#include "skillhelp.h"

class MobSkillData;

class BasicSkill : public virtual Skill, 
                   public XMLTableElement,
                   public XMLVariableContainer 
{
XML_OBJECT
public:
    typedef ::Pointer<BasicSkill> Pointer;

    BasicSkill();

    virtual void loaded( );
    virtual void unloaded( );
    virtual const DLString & getName( ) const;
    virtual void setName( const DLString & );
    virtual const DLString &getRussianName( ) const;
    virtual const DLString& getNameFor( Character * ) const;
    virtual bool matchesStrict( const DLString &str ) const;
    virtual bool matchesUnstrict( const DLString &str ) const;
    virtual bool matchesSubstring( const DLString &str ) const;

    virtual SpellPointer getSpell( ) const;
    virtual AffectHandlerPointer getAffect( ) const;
    virtual SkillCommandPointer getCommand( ) const;
    virtual HelpArticlePointer getSkillHelp( ) const;
    virtual int getBeats(Character *ch = 0) const;
    virtual int getMana(Character *ch = 0) const;
    virtual int getMoves(Character *ch = 0) const;
    virtual int getHealthPenalty() const;
    virtual int getMovesPenalty() const;
    virtual int getManaPenalty() const;
    virtual const RussianString &getDammsg( ) const;
    virtual int getRating( PCharacter * ) const;
    virtual bool isPassive() const;
    virtual bool isValid( ) const
    {
        return true;
    }
    
    virtual int getEffective( Character * ) const;
    virtual int getAdept( PCharacter * ) const;
    virtual void practice( PCharacter * ) const;
    virtual void improve( Character *, bool, Character *victim = NULL, int dam_type = -1, int dam_flags = 0 ) const;
    virtual int getMaximum( Character * ) const;
    virtual MobSkillData *getMobSkillData();

    // Online editing helpers.
    virtual bool accessFromString(const DLString &newValue, ostringstream &errBuf);
    virtual DLString accessToString() const;
    map<DLString, int> parseAccessTokens(const DLString &newValue, const GlobalRegistryBase *registry, ostringstream &errBuf) const;

    // Autohelp rendering.

    /** Format help article for 'other' skill. This method is overriden for most skill types. */
    virtual void show( PCharacter *, ostream & buf ) const;

    /** Output bonus/penalty to skill level. */
    DLString printLevelBonus(PCharacter *ch) const;
    
    /** Print wait state, targets and mana cost for a skill. */
    DLString printWaitAndMana(PCharacter *ch) const;

    /** Print out info about mobs-practicers for this skill. */
    DLString printPracticers(PCharacter *ch) const;

    XML_VARIABLE XMLMultiString name;
    XML_VARIABLE XMLRussianString dammsg;
    XML_VARIABLE XMLIntegerNoEmpty beats;
    XML_VARIABLE XMLIntegerNoEmpty mana;
    XML_VARIABLE XMLIntegerNoEmpty move;
    XML_VARIABLE XMLIntegerNoEmpty manaPenalty, movesPenalty, healthPenalty;
    XML_VARIABLE XMLIntegerNoEmpty hard;
    XML_VARIABLE XMLFlagsNoEmpty   align, ethos;

    XML_VARIABLE XMLPointerNoEmpty<AffectHandler> affect;
    XML_VARIABLE XMLPointerNoEmpty<Spell> spell;
    XML_VARIABLE XMLPointerNoEmpty<SkillCommand> command;
    XML_VARIABLE XMLPointerNoEmpty<SkillHelp> help;
};

#endif

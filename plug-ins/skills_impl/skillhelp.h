/* $Id$
 *
 * ruffina, 2004
 */
#ifndef SKILLHELP_H
#define SKILLHELP_H

#include "markuphelparticle.h"
#include "helpformatter.h"
#include "skillaction.h"
#include "skillcommand.h"
#include "skill.h"

class SkillHelp : public virtual XMLHelpArticle, 
                  public virtual MarkupHelpArticle,
                  public SkillAction 
{
public:
    typedef ::Pointer<SkillHelp> Pointer;

    virtual bool toXML( XMLNode::Pointer& ) const;
    virtual void setSkill( SkillPointer );
    virtual void unsetSkill( );
    inline virtual SkillPointer getSkill( ) const;

    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void applyFormatter( Character *, ostringstream &, ostringstream & ) const;

    SkillPointer skill;
};

inline SkillPointer SkillHelp::getSkill( ) const
{
    return skill;
}

inline const DLString & SkillHelp::getType( ) const
{
    return TYPE;
}

class SkillHelpFormatter : public HelpFormatter {
public:
    SkillHelpFormatter( const char *, Skill::Pointer );
    virtual ~SkillHelpFormatter( );

protected:
    virtual void reset( );
    virtual void setup( Character * );
    virtual bool handleKeyword( const DLString &, ostringstream & );

    Skill::Pointer skill;
    SkillCommand::Pointer cmd;
    bool fRusCmd, fRusSkill;
};

#endif


/* $Id$
 *
 * ruffina, 2004
 */
#ifndef SKILLGROUPHELP_H
#define SKILLGROUPHELP_H

#include "markuphelparticle.h"
#include "skillgroup.h"

class SkillGroupHelp : public MarkupHelpArticle, public SkillGroupAction {
public:
    typedef ::Pointer<SkillGroupHelp> Pointer;

    virtual void setSkillGroup( SkillGroupPointer );
    virtual void unsetSkillGroup( );
    inline virtual SkillGroupPointer getSkillGroup( ) const;
    virtual void save() const;
    virtual DLString getTitle(const DLString &label) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    virtual void getRawText( Character *, ostringstream & ) const;

    SkillGroup::Pointer group;
};

inline SkillGroup::Pointer SkillGroupHelp::getSkillGroup( ) const
{
    return group;
}

inline const DLString & SkillGroupHelp::getType( ) const
{
    return TYPE;
}


#endif

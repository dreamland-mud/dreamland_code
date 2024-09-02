/* $Id$
 *
 * ruffina, 2004
 */
#ifndef AREAHELP_H
#define AREAHELP_H

#include "markuphelparticle.h"

class AreaHelp : public MarkupHelpArticle {
XML_OBJECT
public:
    typedef ::Pointer<AreaHelp> Pointer;

    virtual DLString getTitle(const DLString &label) const;
    virtual void getRawText( Character *, ostringstream & ) const;
    virtual void save() const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;
    
    bool selfHelp;
    bool persistent;
};

inline const DLString & AreaHelp::getType( ) const
{
    return TYPE;
}

struct AreaIndexData;

/** Get self-help article for this area, either a real one or automatically created. */
AreaHelp * area_selfhelp(struct AreaIndexData *area);

/** Get self-help ID for this area, or -1. */
int area_helpid(struct AreaIndexData *area);

/** Return true if this article is empty or consists only of spaces. */
bool help_is_empty(const HelpArticle &help);

#endif

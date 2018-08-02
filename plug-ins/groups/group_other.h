/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __GROUP_OTHER_H__
#define __GROUP_OTHER_H__

#include "genericskill.h"

class ExoticSkill : public GenericSkill {
XML_OBJECT
public:
    typedef ::Pointer<ExoticSkill> Pointer;

    virtual bool visible( Character * ) const;
    virtual bool available( Character * ) const;
    virtual bool usable( Character *, bool ) const;
    virtual int getLearned( Character *ch ) const;
};

#endif

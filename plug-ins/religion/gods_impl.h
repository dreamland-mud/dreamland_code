/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __GODS_IMPL_H__
#define __GODS_IMPL_H__

#include "defaultreligion.h"


class ErevanGod : public DefaultReligion {
XML_OBJECT
public:
        typedef ::Pointer<ErevanGod> Pointer;
        virtual void tattooFight( Object *, Character * ) const;
};

#endif

/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __RIDING_OBJECTS_H__
#define __RIDING_OBJECTS_H__

#include "xmlboolean.h"
#include "objectbehavior.h"

#include "mobiles.h"

class HorseHarness : public virtual ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<HorseHarness> Pointer;

    HorseHarness( );

    virtual bool canDress( Character *, Character * );
};

class HorseBridle : public HorseHarness {
XML_OBJECT
public:
    typedef ::Pointer<HorseBridle> Pointer;

    HorseBridle( );
    
    inline bool isTethered( ) const;

protected:
    XML_VARIABLE XMLBoolean tethered;
};

inline bool HorseBridle::isTethered( ) const
{
    return tethered.getValue( );
}

#endif

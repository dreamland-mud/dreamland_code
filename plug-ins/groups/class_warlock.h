/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __CLASS_WARLOCK_H__
#define __CLASS_WARLOCK_H__

#include "objectbehaviormanager.h"

class EnergyShield : public BasicObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<EnergyShield> Pointer;

    virtual void wear( Character * );
    virtual void equip( Character * );                           
    virtual void remove( Character * );

protected:
    bool isColdShield( ) const;
    bool isFireShield( ) const;
};


#endif

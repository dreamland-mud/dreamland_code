/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __REPAIRMAN_H__
#define __REPAIRMAN_H__

#include "xmlflags.h"
#include "commandplugin.h"
#include "defaultcommand.h"
#include "basicmobilebehavior.h"

class Character;

class Repairman : public virtual BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<Repairman> Pointer;
    
    Repairman( );

    virtual void doRepair( Character *, const DLString & );
    virtual void doEstimate( Character *, const DLString & );

    virtual int getOccupation( );

protected:
    virtual bool specIdle( );

    XML_VARIABLE XMLFlagsNoEmpty repairs;
private:
    int getRepairCost( Object * );
    bool canRepair( Object *, Character * );
};


#endif

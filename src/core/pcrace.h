/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PCRACE_H__
#define __PCRACE_H__

#include "race.h"

class Profession;
class GlobalArray;
class EnumerationArray;

/*
 * PCRace
 */
class PCRace : public virtual Race {
public:	
    typedef ::Pointer<PCRace> Pointer;
    
    PCRace( );
    PCRace( const DLString & );
    virtual ~PCRace( );

    virtual PCRace * getPC( );
    virtual bool  isPC( ) const;

    virtual const EnumerationArray & getStats( ) const;
    virtual GlobalArray & getClasses( );
    virtual GlobalArray & getEquipment( );
    virtual const Flags & getAlign( ) const;
    virtual int getMinAlign( ) const;
    virtual int getMaxAlign( ) const;
    virtual int getPoints( ) const;
    virtual int getHpBonus( ) const;
    virtual int getManaBonus( ) const;
    virtual int getPracBonus( ) const;
    virtual int getSpBonus( ) const;
    
    virtual DLString getWhoNameFor( Character *looker, Character *owner = NULL ) const;
    virtual DLString getScoreNameFor( Character *looker, Character *owner = NULL ) const;
};

#endif

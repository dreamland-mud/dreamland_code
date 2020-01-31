/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __PET_H__
#define __PET_H__

#include "article.h"
#include "price.h"
#include "basicmobilebehavior.h"

class Pet : public Article, public virtual BasicMobileDestiny, public MoneyPrice {
XML_OBJECT
public:
    typedef ::Pointer<Pet> Pointer;
    
    Pet( );

    virtual void stopfol( Character * );

    virtual void purchase( Character *, NPCharacter *, const DLString &, int = 1 );
    virtual bool available( Character *, NPCharacter * ) const;
    virtual int getQuantity( ) const;
    
    virtual int haggle( Character * ) const;
    virtual int getLevel( Character * ) const;
    virtual void toStream( Character *, ostringstream & ) const;
    virtual int toSilver( Character * ) const;
    
    virtual void config( PCharacter *, NPCharacter * ) const;
protected:
    virtual NPCharacter * create( PCharacter * ) const;
};

class LevelAdaptivePet : public Pet {
XML_OBJECT
public:
    typedef ::Pointer<LevelAdaptivePet> Pointer;

    virtual int getLevel( Character * ) const;

    virtual void config( PCharacter *, NPCharacter * ) const;
};

class RideablePet : public Pet {
XML_OBJECT
public:
    typedef ::Pointer<RideablePet> Pointer;
    
    virtual int haggle( Character * ) const;
    virtual void purchase( Character *, NPCharacter *, const DLString &, int = 1 );
    virtual int getOccupation( );

    virtual void config( PCharacter *, NPCharacter * ) const;
protected:
    virtual NPCharacter * create( PCharacter * ) const;
};

#endif

/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __ARCADIAN_BEHAVIORS_H__
#define __ARCADIAN_BEHAVIORS_H__

#include "drinkcontainer.h"
#include "basicmobilebehavior.h"

class DrinkContainerWEBase;

/*
 * ArcadianDrinkBehavior: behavior for container of interesting liquids
 */
class ArcadianDrinkBehavior : public DrinkContainer {
XML_OBJECT    
public:
    typedef ::Pointer<ArcadianDrinkBehavior> Pointer;    
    
    virtual void pourOut( Character *, int );
    virtual void pourOut( Character *, Character *, int );
    virtual void drink( Character *, int );
    virtual void fill( Character *, Object *, int );
    virtual void pour( Character *, Object *, int );
    
    void cleanup( );
    void setEffectName( const WordEffect::Pointer );
    void setQuality( PCharacter * );
    inline int getQuality( ) const {
        return quality.getValue( );
    }
    inline bool isActive( ) const {
        return !effectName.getValue( ).empty( );
    }

protected:
    ::Pointer<DrinkContainerWEBase> findDrinkEffect( ) const;
    bool hasSameEffect( Object * );

    XML_VARIABLE XMLString effectName;
    XML_VARIABLE XMLInteger quality;
};

/*
 * beer elemental
 */
#define MOB_VNUM_BEER_ELEMENTAL 6

class BeerElementalBehavior : public BasicMobileDestiny {
XML_OBJECT
public:
    typedef ::Pointer<BeerElementalBehavior> Pointer;

    virtual bool area( );

protected:
    virtual bool specFight( );
};

#endif

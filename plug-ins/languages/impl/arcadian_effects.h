/* $Id$
 *
 * ruffina, 2009
 */
#ifndef __ARCADIAN_EFFECTS_H__
#define __ARCADIAN_EFFECTS_H__

#include "wordeffect.h"
#include "xmlboolean.h"
#include "arcadian_behaviors.h"

class NPCharacter;
class DrinkContainerWEBase;

/*
 * LiquidWEBase: basic effect for all arcadian words
 */
class LiquidWEBase : public WordEffect {
public:
    typedef ::Pointer<LiquidWEBase> Pointer;

protected:
    bool checkItemType( PCharacter *, Object * ) const;
    bool checkVolume( PCharacter *, Object * ) const;
    bool checkWater( PCharacter *, Object * ) const;
};

/*
 * water2wine and water2beer effects
 */
class WaterToWineWE : public LiquidWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WaterToWineWE> Pointer;
    
    virtual bool run( PCharacter *, Object * ) const;
};

class WaterToBeerWE : public LiquidWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WaterToBeerWE> Pointer;
    
    virtual bool run( PCharacter *, Object * ) const;
};

/*
 * DrinkContainerWEBase: base for effects dealing with drink containers
 */
class DrinkContainerWEBase : public LiquidWEBase {
public:
    typedef ::Pointer<DrinkContainerWEBase> Pointer;
    
    DrinkContainerWEBase( );

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const = 0;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const = 0;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const = 0;

protected:
    bool checkContainer( PCharacter *, Object * ) const;
    void setupBehavior( PCharacter *, Object *) const;

    bool goodQuality( ArcadianDrinkBehavior::Pointer ) const;
    bool goodVolume( int ) const;
    
    XML_VARIABLE XMLInteger minEffectiveVolume;
};

/*
 * WineContainerWEBase: base for effects dealing with wine containers
 */
class WineContainerWEBase : public DrinkContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WineContainerWEBase> Pointer;

    virtual bool run( PCharacter *, Object * ) const;
};

/*
 * wine_refresh, wine_awake, wine_sleep, wine_calm effects
 */
class WineRefreshWE : public WineContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WineRefreshWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
};

class WineAwakeWE : public WineContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WineAwakeWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
};

class WineSleepWE : public WineContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WineSleepWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
};

class WineCalmWE : public WineContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<WineCalmWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
};

/*
 * BeerContainerWEBase: base for effects dealing with beer containers
 */
class BeerContainerWEBase : public DrinkContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<BeerContainerWEBase> Pointer;

    virtual bool run( PCharacter *, Object * ) const;
};

/*
 * beer_armor, beer_elemental effects
 */
class BeerArmorWE : public BeerContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<BeerArmorWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
};

class BeerElementalWE : public BeerContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<BeerElementalWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const;
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const;
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const;

private:
    NPCharacter *createElemental( PCharacter *, ArcadianDrinkBehavior::Pointer ) const;
};

#endif

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
 * DrinkContainerWEBase: base for effects dealing with drink containers.
 *
 * The concrete arcadian word-effects (water2wine/beer, wine_refresh/sleep/
 * awake/calm, beer_armor/elemental) were ported to Fenia -- see
 * dreamland_fenia/wordeffect/arcadian/ and utils/arcadia. languagecommand.cpp
 * gives the Fenia runObj first crack on utter, and the drink/pour instance
 * triggers (code#1025/#1029/#1030) carry the pour/drink payload, so no C++
 * effect logic runs anymore. Only FeniaWordEffectWE below stays as the single
 * instantiable stub. This base is kept because ArcadianDrinkBehavior still
 * dispatches through its virtuals for any container that carried the old C++
 * behavior (now a no-op path).
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
 * FeniaWordEffectWE: the sole concrete arcadian word-effect. Every <effects>
 * node in arcadian.xml instantiates this stub; the real behaviour lives in
 * Fenia (runFeniaEffect intercepts the utter before run(), and the pour/drink
 * hooks fire the instance triggers). These C++ bodies are unreachable fallbacks
 * kept only so the class is concrete and XML-instantiable, and so an old saved
 * container that still carries ArcadianDrinkBehavior degrades to a harmless
 * no-op instead of crashing.
 */
class FeniaWordEffectWE : public DrinkContainerWEBase {
XML_OBJECT
public:
    typedef ::Pointer<FeniaWordEffectWE> Pointer;

    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, int ) const { }
    virtual void onPourOut( ArcadianDrinkBehavior::Pointer, Character *, Character *, int ) const { }
    virtual void onDrink( ArcadianDrinkBehavior::Pointer, Character *, int ) const { }
};

#endif

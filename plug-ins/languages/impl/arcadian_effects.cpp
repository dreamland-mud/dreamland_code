/* $Id$
 *
 * ruffina, 2009
 */
#include "arcadian_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "race.h"
#include "liquid.h"
#include "liquidflags.h"

#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "act.h"
#include "loadsave.h"

#include "vnum.h"
#include "def.h"
#include "l10n.h"

GSN(arcadian);
LIQ(water);

/*
 * LiquidWEBase
 */
bool LiquidWEBase::checkItemType( PCharacter *ch, Object *obj ) const
{
    if (obj->item_type != ITEM_DRINK_CON) {
        ch->pecho(_("%1$^O1 мало похож%1$Gе||а|и на емкость для жидкости."), obj);
        return false;
    }

    return true;
}

bool LiquidWEBase::checkVolume( PCharacter *ch, Object *obj ) const
{
    if (obj->value1() == 0) {
        oldact(_("Слово эхом отозвалось в пустоте $o2."), ch, obj, 0, TO_CHAR);
        return false;
    }

    if (obj->value1() > gsn_arcadian->getEffective( ch ) * 10) {
        oldact(_("В $o6 налито слишком много жидкости."), ch, obj, 0, TO_CHAR);
        return false;
    }

    return true;
}

bool LiquidWEBase::checkWater( PCharacter *ch, Object *obj ) const
{
    if (obj->value2() != liq_water) {
        oldact(_("Это слово действует только на воду."), ch, 0, 0, TO_CHAR);
        return false;
    }

    return true;
}

/*
 * DrinkContainerWEBase
 */
DrinkContainerWEBase::DrinkContainerWEBase( )
{
}

void DrinkContainerWEBase::setupBehavior( PCharacter *ch, Object *obj ) const
{
    if (obj->behavior)
        obj->behavior->unsetObj( );

    ArcadianDrinkBehavior::Pointer bhv( NEW );
    bhv->setEffectName( Pointer( this ) );
    bhv->setQuality( ch );
    bhv->setObj( obj );
    obj->behavior.setPointer( *bhv );
}

bool DrinkContainerWEBase::checkContainer( PCharacter *ch, Object *obj ) const
{
    ArcadianDrinkBehavior::Pointer arcadBehavior;

    if (!obj->behavior)
        return true;

    // Can't apply arcadian words to a container with complex behavior.
    arcadBehavior = obj->behavior.getDynamicPointer<ArcadianDrinkBehavior>();
    if (!arcadBehavior && obj->behavior->getType() != "BasicObjectBehavior") {
        oldact(_("Повлиять на эту емкость у тебя не получится."), ch, obj, 0, TO_CHAR);
        return false;
    }

    if (arcadBehavior && arcadBehavior->isActive( )) {
        oldact(_("Жидкость в $o6 уже обладает необычными свойствами."), ch, obj, 0, TO_CHAR);
        return false;
    }

    return true;
}

bool DrinkContainerWEBase::goodQuality( ArcadianDrinkBehavior::Pointer bhv ) const
{
    return number_percent( ) < bhv->getQuality( );
}

bool DrinkContainerWEBase::goodVolume( int amount ) const
{
    return amount >= minEffectiveVolume.getValue( );
}

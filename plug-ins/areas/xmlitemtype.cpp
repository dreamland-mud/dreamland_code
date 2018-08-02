/* $Id$
 *
 * ruffina, 2004
 */

#include "xmlitemtype.h"
#include "itemflags.h"
#include "autoflags.h"

bool 
XMLItemType::toXML(XMLNode::Pointer &parent) const
{
    switch(type) {
    case ITEM_MONEY:
        {
            XMLItemTypeValuesMoney tmp;
            tmp.silver.setValue(v[0]);
            tmp.gold.setValue(v[1]);
            tmp.reserved0.setValue(v[2]);
            tmp.reserved1.setValue(v[3]);
            tmp.reserved2.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_FOUNTAIN:
    case ITEM_DRINK_CON:
        {
            XMLItemTypeValuesDrink tmp;
            tmp.total.setValue(v[0]);
            tmp.left.setValue(v[1]);
            tmp.liquid = v[2];
            tmp.flags.setValue(v[3]);
            tmp.corkscrew.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_CONTAINER:
        {
            XMLItemTypeValuesContainer tmp;
            tmp.capacity.setValue(v[0]);
            tmp.flags.setValue(v[1]);
            tmp.key.setValue(v[2]);
            tmp.maxWeight.setValue(v[3]);
            tmp.weightMult.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_BOAT:
        {
            XMLItemTypeValuesBoat tmp;
            tmp.move_type.setValue(v[0]);
            tmp.position.setValue(v[1]);
            tmp.flags.setValue(v[2]);
            tmp.reserved0.setValue(v[3]);
            tmp.reserved1.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_FOOD:
        {
            XMLItemTypeValuesFood tmp;
            tmp.foodHours.setValue(v[0]);
            tmp.fullHours.setValue(v[1]);
            tmp.reserved0.setValue(v[2]);
            tmp.poisoned.setValue(v[3]);
            tmp.reserved1.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_PORTAL:
        {
            XMLItemTypeValuesPortal tmp;
            tmp.charges.setValue(v[0]);
            tmp.exitFlags.setValue(v[1]);
            tmp.portalFlags.setValue(v[2]);
            tmp.target.setValue(v[3]);
            tmp.key.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_FURNITURE:
        {
            XMLItemTypeValuesFurniture tmp;
            tmp.maxPeople.setValue(v[0]);
            tmp.maxWeight.setValue(v[1]);
            tmp.flags.setValue(v[2]);
            tmp.heal.setValue(v[3]);
            tmp.mana.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_WEAPON:
        {
            XMLItemTypeValuesWeapon tmp;
            tmp.weapClass.setValue(v[0]);
            tmp.diceNumber.setValue(v[1]);
            tmp.diceType.setValue(v[2]);
            tmp.weapType.setValue(v[3]);
            tmp.specType.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_ARMOR:
        {
            XMLItemTypeValuesArmor tmp;
            tmp.ac.pierce = v[0];
            tmp.ac.bash = v[1];
            tmp.ac.slash = v[2];
            tmp.ac.exotic = v[3];
            tmp.reserved0.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        {
            XMLItemTypeValuesScrollPotionPill tmp;
            tmp.level.setValue(v[0]);
            tmp.spell0 = v[1];
            tmp.spell1 = v[2];
            tmp.spell2 = v[3];
            tmp.spell3 = v[4];
            tmp.toXML(parent);
        }
        break;

    case ITEM_STAFF:
    case ITEM_WAND:
        {
            XMLItemTypeValuesStaffWand tmp;
            tmp.level.setValue(v[0]);
            tmp.total.setValue(v[1]);
            tmp.left.setValue(v[2]);
            tmp.spell = v[3];
            tmp.reserved0.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_SPELLBOOK:
    case ITEM_TEXTBOOK:
        {
            XMLItemTypeValuesSpellBook tmp;
            tmp.total.setValue(v[0]);
            tmp.used.setValue(v[1]);
            tmp.quality.setValue(v[2]);
            tmp.skill0 = v[3];
            tmp.skill1 = v[4];
            tmp.toXML(parent);
        }
        break;

    case ITEM_KEY:
        {
            XMLItemTypeValuesKey tmp;
            tmp.rotInv.setValue(v[0]);
            tmp.rotGround.setValue(v[1]);
            tmp.reserved0.setValue(v[2]);
            tmp.reserved1.setValue(v[3]);
            tmp.reserved2.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    case ITEM_KEYRING:
        {
            XMLItemTypeValuesKeyring tmp;
            tmp.maxKeys.setValue(v[0]);
            tmp.reserved0.setValue(v[1]);
            tmp.reserved1.setValue(v[2]);
            tmp.reserved2.setValue(v[3]);
            tmp.reserved3.setValue(v[4]);
            tmp.toXML(parent);
        }
        break;

    default:
        {
            XMLItemTypeValuesDefault tmp;
            tmp.v0.setValue(v[0]);
            tmp.v1.setValue(v[1]);
            tmp.v2.setValue(v[2]);
            tmp.v3.setValue(v[3]);
            tmp.v4.setValue(v[4]);
            tmp.toXML(parent);
        }
    }
    parent->insertAttribute("name", item_table.name(type));
    return true;
}

void
XMLItemType::fromXML(const XMLNode::Pointer &parent) throw(ExceptionBadType)
{
    type = item_table.value(parent->getAttribute("name"));

    switch(type) {
    case ITEM_MONEY:
        {
            XMLItemTypeValuesMoney tmp;
            tmp.fromXML(parent);
            v[0] = tmp.silver.getValue( );
            v[1] = tmp.gold.getValue( );
            v[2] = tmp.reserved0.getValue( );
            v[3] = tmp.reserved1.getValue( );
            v[4] = tmp.reserved2.getValue( );
        }
        break;

    case ITEM_FOUNTAIN:
    case ITEM_DRINK_CON:
        {
            XMLItemTypeValuesDrink tmp;
            tmp.fromXML(parent);
            v[0] = tmp.total.getValue( );
            v[1] = tmp.left.getValue( );
            v[2] = tmp.liquid;
            v[3] = tmp.flags.getValue( );
            v[4] = tmp.corkscrew.getValue( );
        }
        break;

    case ITEM_CONTAINER:
        {
            XMLItemTypeValuesContainer tmp;
            tmp.fromXML(parent);
            v[0] = tmp.capacity.getValue( );
            v[1] = tmp.flags.getValue( );
            v[2] = tmp.key.getValue( );
            v[3] = tmp.maxWeight.getValue( );
            v[4] = tmp.weightMult.getValue( );
        }
        break;

    case ITEM_BOAT:
        {
            XMLItemTypeValuesBoat tmp;
            tmp.fromXML(parent);
            v[0] = tmp.move_type.getValue( );
            v[1] = tmp.position.getValue( );
            v[2] = tmp.flags.getValue( );
            v[3] = tmp.reserved0.getValue( );
            v[4] = tmp.reserved1.getValue( );
        }
        break;

    case ITEM_FOOD:
        {
            XMLItemTypeValuesFood tmp;
            tmp.fromXML(parent);
            v[0] = tmp.foodHours.getValue( );
            v[1] = tmp.fullHours.getValue( );
            v[2] = tmp.reserved0.getValue( );
            v[3] = tmp.poisoned.getValue( );
            v[4] = tmp.reserved1.getValue( );
        }
        break;

    case ITEM_PORTAL:
        {
            XMLItemTypeValuesPortal tmp;
            tmp.fromXML(parent);
            v[0] = tmp.charges.getValue( );
            v[1] = tmp.exitFlags.getValue( );
            v[2] = tmp.portalFlags.getValue( );
            v[3] = tmp.target.getValue( );
            v[4] = tmp.key.getValue( );
        }
        break;

    case ITEM_FURNITURE:
        {
            XMLItemTypeValuesFurniture tmp;
            tmp.fromXML(parent);
            v[0] = tmp.maxPeople.getValue( );
            v[1] = tmp.maxWeight.getValue( );
            v[2] = tmp.flags.getValue( );
            v[3] = tmp.heal.getValue( );
            v[4] = tmp.mana.getValue( );
        }
        break;

    case ITEM_WEAPON:
        {
            XMLItemTypeValuesWeapon tmp;
            tmp.fromXML(parent);
            v[0] = tmp.weapClass.getValue( );
            v[1] = tmp.diceNumber.getValue( );
            v[2] = tmp.diceType.getValue( );
            v[3] = tmp.weapType.getValue( );
            v[4] = tmp.specType.getValue( );
        }
        break;

    case ITEM_ARMOR:
        {
            XMLItemTypeValuesArmor tmp;
            tmp.fromXML(parent);
            v[0] = tmp.ac.pierce;
            v[1] = tmp.ac.bash;
            v[2] = tmp.ac.slash;
            v[3] = tmp.ac.exotic;
            v[4] = tmp.reserved0.getValue( );
        }
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        {
            XMLItemTypeValuesScrollPotionPill tmp;
            tmp.fromXML(parent);
            v[0] = tmp.level.getValue( );
            v[1] = tmp.spell0;
            v[2] = tmp.spell1;
            v[3] = tmp.spell2;
            v[4] = tmp.spell3;
        }
        break;

    case ITEM_STAFF:
    case ITEM_WAND:
        {
            XMLItemTypeValuesStaffWand tmp;
            tmp.fromXML(parent);
            v[0] = tmp.level.getValue( );
            v[1] = tmp.total.getValue( );
            v[2] = tmp.left.getValue( );
            v[3] = tmp.spell;
            v[4] = tmp.reserved0.getValue( );
        }
        break;

    case ITEM_SPELLBOOK:
    case ITEM_TEXTBOOK:
        {
            XMLItemTypeValuesSpellBook tmp;
            tmp.fromXML(parent);
            v[0] = tmp.total.getValue( );
            v[1] = tmp.used.getValue( );
            v[2] = tmp.quality.getValue( );
            v[3] = tmp.skill0;
            v[4] = tmp.skill1;
        }
        break;

    case ITEM_KEY:
        {
            XMLItemTypeValuesKey tmp;
            tmp.fromXML(parent);
            v[0] = tmp.rotInv.getValue( );
            v[1] = tmp.rotGround.getValue( );
            v[2] = tmp.reserved0.getValue( );
            v[3] = tmp.reserved1.getValue( );
            v[4] = tmp.reserved2.getValue( );
        }
        break;

    case ITEM_KEYRING:
        {
            XMLItemTypeValuesKeyring tmp;
            tmp.fromXML(parent);
            v[0] = tmp.maxKeys.getValue( );
            v[1] = tmp.reserved0.getValue( );
            v[2] = tmp.reserved1.getValue( );
            v[3] = tmp.reserved2.getValue( );
            v[4] = tmp.reserved3.getValue( );
        }
        break;

    default:
        {
            XMLItemTypeValuesDefault tmp;
            tmp.fromXML(parent);
            v[0] = tmp.v0.getValue( );
            v[1] = tmp.v1.getValue( );
            v[2] = tmp.v2.getValue( );
            v[3] = tmp.v3.getValue( );
            v[4] = tmp.v4.getValue( );
        }
    }
}

/***************************************************************
 * values
 ***************************************************************/
XMLItemTypeValuesFood::XMLItemTypeValuesFood( ) : poisoned(false) 
{
}

XMLItemTypeValuesDrink::XMLItemTypeValuesDrink( ) : flags(0, &drink_flags)
{
}

XMLItemTypeValuesContainer::XMLItemTypeValuesContainer( ) : flags(0, &container_flags)
{
}

XMLItemTypeValuesBoat::XMLItemTypeValuesBoat( ) : 
            position(POS_STANDING, &position_table), flags(0, &furniture_flags)
{
}

XMLItemTypeValuesPortal::XMLItemTypeValuesPortal( ) : exitFlags(0, &exit_flags), portalFlags(0, &portal_flags)
{
}

XMLItemTypeValuesFurniture::XMLItemTypeValuesFurniture( ) : flags(0, &furniture_flags)
{
}

XMLItemTypeValuesWeapon::XMLItemTypeValuesWeapon( ) : 
            weapClass(WEAPON_EXOTIC, &weapon_class), 
            weapType(DAMW_NONE, &weapon_flags), 
            specType(0, &weapon_type2)
{
}

/*
 * Item-set behavior implementation: perma-affects engine (#2758), phase 3.
 */
#include "setbehavior.h"

#include "affect.h"
#include "affectflags.h"
#include "merc.h"

#include <algorithm>

/*********************************************************************
 * SetApply -- mirror of areas' XMLApply
 *********************************************************************/
SetApply::SetApply( ) : location( APPLY_NONE )
{
}

bool
SetApply::toXML( XMLNode::Pointer &parent ) const
{
    if (location == APPLY_NONE && getValue( ) == 0)
        return false;

    if (!XMLIntegerNoEmpty::toXML( parent ))
        return false;

    parent->insertAttribute( "to", apply_flags.name( location ) );
    return true;
}

void
SetApply::fromXML( const XMLNode::Pointer &parent )
{
    location = apply_flags.value( parent->getAttribute( "to" ) );
    XMLIntegerNoEmpty::fromXML( parent );
}

/*********************************************************************
 * SetAffect
 *********************************************************************/
void
SetAffect::fill( Affect &af ) const
{
    af.bitvector.setTable( bits.getTable( ) );
    af.bitvector.setValue( bits.getValue( ) );
    af.global.setRegistry( global.getRegistry( ) );
    af.global.set( global );
    af.location.setTable( &apply_flags );
    af.location = apply.location;
    af.modifier = apply.getValue( );
}

void
SetAffect::fillScaled( Affect &af, int level, int refLevel ) const
{
    fill( af );

    if (refLevel <= 0 || level <= 0 || af.modifier == 0)
        return;

    switch (apply.location) {
    case APPLY_HIT: case APPLY_MANA: case APPLY_MOVE:
    case APPLY_DAMROLL: case APPLY_HITROLL: case APPLY_AC:
    case APPLY_SAVES: case APPLY_SAVING_SPELL:
    case APPLY_HEAL_GAIN: case APPLY_MANA_GAIN:
        break;
    default:
        return;
    }

    int v = af.modifier;
    int a = (v < 0 ? -v : v);
    int scaled = std::max( 1, (a * level + refLevel / 2) / refLevel );
    af.modifier = (v < 0 ? -scaled : scaled);
}

/*********************************************************************
 * SetBehavior
 *********************************************************************/
int
SetBehavior::getRefLevel( ) const
{
    if (!props.isObject( ) || !props.isMember( "ref_level" ))
        return 0;
    const Json::Value &v = props["ref_level"];
    return v.isInt( ) ? v.asInt( ) : 0;
}

const DLString &
SetBehavior::getMsgComplete( lang_t lang ) const
{
    return msgComplete.getForLang( lang );
}

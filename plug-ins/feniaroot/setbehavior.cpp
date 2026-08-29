/*
 * Item-set behavior implementation: perma-affects engine (#2758), phase 3.
 */
#include "setbehavior.h"

#include "affect.h"
#include "affectflags.h"
#include "merc.h"

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

/*********************************************************************
 * SetBehavior
 *********************************************************************/
const DLString &
SetBehavior::getMsgComplete( lang_t lang ) const
{
    return msgComplete.getForLang( lang );
}

/* $Id$
 *
 * ruffina, 2004
 */
#ifndef WEARLOC_UTILS
#define WEARLOC_UTILS

#include "wearlocation.h"
#include "wearloc_codes.h"

// macros for historical compat
#define get_eq_char(ch, iWear)	    wearlocationManager->find(iWear)->find( ch )
#define equip_char(ch, obj, iWear)  (iWear)->equip( obj )
#define unequip_char(ch, obj)	    obj->wear_loc->unequip( obj )
#define wear_obj(ch, obj, flags )   wearlocationManager->wear( obj, flags )

// most commonly used wearlocs 
#ifndef _WEARLOC_
#   define _WEARLOC_( name ) extern WearlocationReference wear_##name;
#endif

_WEARLOC_( none );
_WEARLOC_( light );
_WEARLOC_( shield );
_WEARLOC_( hold );
_WEARLOC_( tattoo );
_WEARLOC_( wield );
_WEARLOC_( second_wield );
_WEARLOC_( stuck_in );
_WEARLOC_( hold_leg );
_WEARLOC_( body );
_WEARLOC_( neck_1 );
_WEARLOC_( neck_2 );
_WEARLOC_( float );
_WEARLOC_( wrist_l );
_WEARLOC_( wrist_r );
_WEARLOC_( finger_l );
_WEARLOC_( finger_r );
_WEARLOC_( ears );
_WEARLOC_( head );
_WEARLOC_( arms );
_WEARLOC_( hands );
_WEARLOC_( feet );


#endif

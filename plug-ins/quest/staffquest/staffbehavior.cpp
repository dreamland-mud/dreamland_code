/* $Id: staffbehavior.cpp,v 1.1.2.14.6.2 2008/03/01 20:37:16 rufina Exp $
 *
 * ruffina, 2003
 */

#include "staffbehavior.h"
#include "staffquest.h"

#include "pcharacter.h"
#include "object.h"
#include "skillreference.h"

#include "merc.h"
#include "act.h"
#include "handler.h"
#include "mercdb.h"
#include "magic.h"
#include "def.h"

GSN(curse);
GSN(poison);
GSN(cure_poison);
GSN(remove_curse);

void StaffBehavior::getByOther( Character *ch ) 
{
    short level = max( 1, ch->getModifyLevel( ) - 9 );

    ch->pecho( "%1$O1 не принадлежит тебе, и ты бросаешь %1$P2.", obj );

    switch (dice( 1, 10 ))  {
    case 1:
	spell( gsn_curse, level, ch, ch );
	break;
    case 2:
	spell( gsn_poison, level, ch, ch );
	break;
    }
}

void StaffBehavior::getByHero( PCharacter *ch ) 
{
    if (IS_AFFECTED( ch, AFF_POISON ) && (dice( 1, 5) == 1))  {
	act( "$o1 загорается голубым пламенем.", ch, obj, 0, TO_CHAR );
	spell( gsn_cure_poison, 30, ch, ch );
	return;
    }

    if ( IS_AFFECTED( ch, AFF_CURSE ) && (dice(1,5)==1) )  {
	act( "$o1 загорается голубым пламенем.", ch, obj, 0, TO_CHAR );
	spell( gsn_remove_curse, 30, ch, ch );
	return;
    }
    
    act( "Мерцающая аура окружает $o4.", ch, obj, 0, TO_CHAR );
}


/* $Id: update.h,v 1.1.2.1 2007/09/11 00:03:42 rufina Exp $
 * 
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#ifndef UPDATE_H
#define UPDATE_H

class PCharacter;

void update_handler( );
void char_update( );
void area_update( );
void room_update( );
void obj_update( );
void mobile_update( );
void track_update( );
void player_update( );

#endif

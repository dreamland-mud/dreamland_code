/* $Id$
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
#ifndef _VNUM_H_
#define _VNUM_H_

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE            1
#define OBJ_VNUM_GOLD_ONE              2
#define OBJ_VNUM_GOLD_SOME             3
#define OBJ_VNUM_SILVER_SOME           4
#define OBJ_VNUM_COINS                 5

#define OBJ_VNUM_GRAVE                    9
#define OBJ_VNUM_CORPSE_NPC             10
#define OBJ_VNUM_CORPSE_PC             11
#define OBJ_VNUM_SEVERED_HEAD             12
#define OBJ_VNUM_TORN_HEART             13
#define OBJ_VNUM_SLICED_ARM             14
#define OBJ_VNUM_SLICED_PAW             191
#define OBJ_VNUM_SLICED_LEG             15
#define OBJ_VNUM_GUTS                     16
#define OBJ_VNUM_BRAINS                     17

#define OBJ_VNUM_POTION_VIAL             42

#define OBJ_VNUM_CORK 19 

#define OBJ_VNUM_STEAK               27

#define OBJ_VNUM_KATANA_SWORD              98
#define OBJ_VNUM_HAMMER                        6522
#define OBJ_VNUM_CHUNK_IRON                6521

#define OBJ_VNUM_MONUMENT                105

#define OBJ_VNUM_POOL 75 

#define OBJ_VNUM_WEAPON_STUB 104 // random weapon prototype for rand_all

/*
 * Well known room virtual numbers.
 */
#define ROOM_VNUM_LIMBO                2
#define ROOM_VNUM_JAIL                4
#define ROOM_VNUM_REMORT        6
#define ROOM_VNUM_CHAT                1200
#define ROOM_VNUM_TEMPLE        3001
#define ROOM_VNUM_ALTAR                3054
#define ROOM_VNUM_HARBOUR       4512
#define ROOM_VNUM_BUREAU_1 3078 // historical lost&found room, before 2018
#define ROOM_VNUM_BUREAU_2 3083 // new lost&found room
#define ROOM_VNUM_BUREAU_3 3084 // storage room for old personal chests

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_FIDO                    3062

#endif /* !_VNUM_H_ */

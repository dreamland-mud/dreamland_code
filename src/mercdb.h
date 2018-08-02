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
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#ifndef _MERCDB_H_
#define _MERCDB_H_

#include "config_io.h"
#include "dl_ctype.h"
#include "dl_math.h"
#include "dl_strings.h"
#include "logstream.h"

class Object;
class NPCharacter;
class Character;
class Room;
struct mob_index_data;
struct obj_index_data;
struct extra_descr_data;
struct area_data;

#define	MAX_KEY_HASH		 1024

extern char str_empty[1];

/* vals from db.c */
extern bool fBootDb;
extern int mobile_count;
extern int		newmobs;
extern int		newobjs;
extern mob_index_data 	* mob_index_hash          [MAX_KEY_HASH];
extern obj_index_data 	* obj_index_hash          [MAX_KEY_HASH];
extern area_data 	* area_first;
extern Room * room_list;

extern int	top_affect;
extern int	top_area;
extern int	top_ed;
extern int	top_exit;
extern int	top_mob_index;
extern int	top_obj_index;
extern int	top_reset;
extern int	top_room;

// MOC_SKIP_BEGIN
struct area_file {
    struct area_file *next;
    struct area_data *area;
    char *file_name;
};

extern struct area_file * area_file_list;
struct area_file * new_area_file(const char *name);
// MOC_SKIP_END

/*
 * Memory management.
 * Increase MAX_STRING if you have too.	
 * Tune the others only if you understand what you're doing.
 */
#define			MAX_STRING	10000000
#define			MAX_PERM_BLOCK	131072
#define			MAX_MEM_LIST	11
extern int	nAllocString;
extern int	sAllocString;
extern int	nAllocPerm;
extern int	sAllocPerm;

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))

/* Magic number for memory allocation */
#define MAGIC_NUM 52571214

void * alloc_perm(int);
void * alloc_mem(int);
void free_mem(void *, int);

mob_index_data *	get_mob_index	( int vnum );
obj_index_data *	get_obj_index	( int vnum );
Room *	get_room_index	( int vnum );

char *	get_extra_descr	( const char *name, extra_descr_data *ed );
extra_descr_data *new_extra_descr( );
void free_extra_descr( extra_descr_data * );

char *	str_dup		( const char *str );
void	free_string	( char *pstr );


#endif

/* $Id: magic.h,v 1.1.2.7 2008/04/28 23:31:01 rufina Exp $
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
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                           *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael           *
 *  Chastain, Michael Quan, and Mitchell Tse.                                   *
 *                                                                           *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc           *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                                   *
 *                                                                           *
 *  Much time and thought has gone into this software and you are           *
 *  benefitting.  We hope that you share your changes too.  What goes           *
 *  around, comes around.                                                   *
 ***************************************************************************/
 
/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#ifndef _MAGIC_H_
#define _MAGIC_H_

#include "bitstring.h"

class Character;
class Object;
class Room;
class SpellTarget;
class Spell;

bool saves_spell( short level, Character *victim, int dam_type, Character *ch = NULL, bitstring_t dam_flag = 0, bool verbose = true );

/** Effective saves model from fight/saves.json: 0 legacy, 1 curve. */
int saves_model( );

/** Save chance in permille as saves_spell rolls it: no RNG, no messages, no skill improve.
 *  holyCraft = the caller's holy craft roll landed. Caster onSaves hooks are included. */
int saves_chance( short level, Character *victim, int dam_type, Character *ch, bitstring_t dam_flag, bool holyCraft );

/** Save chance in permille from bare numbers. saving_throw is the character field
 *  (negative is better), shift is in percent. model 1 falls back to 0 if not configured. */
int saves_chance_at( int victimLevel, int castLevel, int saving_throw, int shift, int model );

/** While alive, a Fenia vict.saves_spell() without a dam_flag defaults to this spell's
 *  DAMF_MAGIC or DAMF_PRAYER under saves model 1. Set around Fenia spell scripts. */
class SavesSpellScope {
public:
    SavesSpellScope( Spell *spell, Character *caster );
    ~SavesSpellScope( );
    SavesSpellScope( const SavesSpellScope & ) = delete;
    SavesSpellScope &operator = ( const SavesSpellScope & ) = delete;
};

/** DAMF_OTHER under model 0 or outside a spell script, else the running spell's flag. */
bitstring_t saves_default_damflag( );

void attack_caster( Character *caster, Character *victim );
void area_message( Character *ch, const DLString &msg, bool everywhere );

#define    FSPELL_BANE      (A)
#define    FSPELL_VERBOSE   (B)
#define    FSPELL_WAIT      (C)
#define    FSPELL_OBSTACLES (D)
#define    FSPELL_MANA      (E)
#define    FSPELL_NOTRIGGER (F)
#define    FSPELL_ATTACK_CASTER (G)
#define    FSPELL_CHECK_SAFE    (H)
#define    FSPELL_CHECK_GROUP   (I)

bool spell( int sn, int level, Character *ch, ::Pointer<SpellTarget>, int flags = 0 );
bool spell( int sn, int level, Character *ch, Character *victim, int flags = 0 );
bool spell_nocatch( int sn, int level, Character *ch, Character *victim, int flags = 0 );
bool spell( int sn, int level, Character *ch, Object *obj );
bool spell( int sn, int level, Character *ch, Room *room );
bool spell( int sn, int level, Character *ch, char *arg );
bool spell_nocatch( int sn, int level, Character *ch, ::Pointer<SpellTarget>, int flags = 0 );
bool spell_nocatch( ::Pointer<Spell> &spell, int level, Character *ch, ::Pointer<SpellTarget> target, int flags );

/** oathPaid: a Battlerager already rolled his aura for this item, his own aura does not block it again. */
/** oathPaid: a Battlerager paid the oath for this item. burned: the oath burned its magic,
 *  only natural remedies on a pill still work. */
void spell_by_item( Character *ch, Object *obj, bool oathPaid = false, bool burned = false );

/** ownDebuff: the dispeller placed this affect on somebody else (fight/saves.json dispel.own_debuff_pct). */
bool savesDispel( int dis_level, int spell_level, int duration, bool ownDebuff = false );
bool checkDispel( int dis_level, Character *victim, int sn, Character *dispeller = 0 );

bool         is_safe_spell( Character *ch, Character *victim, bool area );

#endif

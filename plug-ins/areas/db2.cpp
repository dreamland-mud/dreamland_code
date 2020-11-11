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
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
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

#include <config.h>

#include "fileformatexception.h"
#include "logstream.h"
#include "grammar_entities_impl.h"

#include "core/fenia/feniamanager.h"
#include "fenia/register-impl.h"

#include "mobilebehaviormanager.h"
#include "mobilebehavior.h"
#include "objectbehaviormanager.h"
#include "objectbehavior.h"

#include "flags.h"
#include "skillmanager.h"
#include "liquid.h"
#include "affect.h"
#include "race.h"

#include "loadsave.h"
#include "fread_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(none);

bool dup_mob_vnum( int vnum );
bool dup_obj_vnum( int vnum );

/*
 * parse one mobile. no explicit side effects
 */
void convert_shop( MOB_INDEX_DATA *pMobIndex, MobileBehavior::Pointer shopper );
MobileBehavior::Pointer allocate_shop( );

void
load_mobile(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
    Race *race;
    MobileBehavior::Pointer shopper;
    const char *word;

    pMobIndex->wrapper                = 0;

    pMobIndex->new_format        = true;
    pMobIndex->player_name      = fread_string( fp );
    pMobIndex->short_descr      = fread_string( fp );
    pMobIndex->long_descr       = fread_string( fp );
    pMobIndex->description      = fread_string( fp );
    pMobIndex->race                = fread_string( fp );
    race = raceManager->find( pMobIndex->race );
    
    pMobIndex->act              = fread_flag( fp ) | ACT_IS_NPC | race->getAct( );
    pMobIndex->affected_by      = fread_flag( fp ) | race->getAff( );
    pMobIndex->practicer.clear( );
    pMobIndex->religion.clear( );
    pMobIndex->detection        = race->getDet( );

    pMobIndex->alignment        = fread_number( fp );
    pMobIndex->group            = fread_number( fp );

    pMobIndex->level            = fread_number( fp );
    pMobIndex->hitroll          = fread_number( fp );  

    /* read hit dice */
    pMobIndex->hit[DICE_NUMBER] = fread_number( fp );  
    /* 'd'          */            fread_letter( fp ); 
    pMobIndex->hit[DICE_TYPE]   = fread_number( fp );
    /* '+'          */            fread_letter( fp );   
    pMobIndex->hit[DICE_BONUS]  = fread_number( fp ); 

    /* read mana dice */
    pMobIndex->mana[DICE_NUMBER]= fread_number( fp );
                                  fread_letter( fp );
    pMobIndex->mana[DICE_TYPE]        = fread_number( fp );
                                  fread_letter( fp );
    pMobIndex->mana[DICE_BONUS]        = fread_number( fp );

    /* read damage dice */
    pMobIndex->damage[DICE_NUMBER]  = fread_number( fp );
                                      fread_letter( fp );
    pMobIndex->damage[DICE_TYPE]    = fread_number( fp );
                                      fread_letter( fp );
    pMobIndex->damage[DICE_BONUS]   = fread_number( fp );
    word = fread_word(fp);
    if (( pMobIndex->dam_type = weapon_flags.value(word) ) == NO_FLAG) 
        throw FileFormatException("load_mobile %d bad dam_type %s", pMobIndex->vnum, word);

    /* read armor class */
    pMobIndex->ac[AC_PIERCE]        = fread_number( fp ) * 10;
    pMobIndex->ac[AC_BASH]        = fread_number( fp ) * 10;
    pMobIndex->ac[AC_SLASH]        = fread_number( fp ) * 10;
    pMobIndex->ac[AC_EXOTIC]        = fread_number( fp ) * 10;

    /* read flags and add in data from the race table */
    pMobIndex->off_flags        = fread_flag( fp ) | race->getOff( );
    pMobIndex->imm_flags        = fread_flag( fp ) | race->getImm( );
    pMobIndex->res_flags        = fread_flag( fp ) | race->getRes( );
    pMobIndex->vuln_flags        = fread_flag( fp ) | race->getVuln( );

    /* vital statistics */
    if (( pMobIndex->start_pos        = position_table.value(fread_word(fp)) ) == NO_FLAG)
        throw FileFormatException("load_mobile %d bad start_pos", pMobIndex->vnum);
    if (( pMobIndex->default_pos = position_table.value(fread_word(fp)) ) == NO_FLAG)
        throw FileFormatException("load_mobile %d bad default_pos", pMobIndex->vnum);
    if (( pMobIndex->sex = sex_table.value(fread_word(fp)) ) == NO_FLAG)
        throw FileFormatException("load_mobile %d bad sex", pMobIndex->vnum);

    pMobIndex->wealth                = fread_number( fp );

    pMobIndex->form                = fread_flag( fp ) | race->getForm( );
    pMobIndex->parts                = fread_flag( fp ) | race->getParts( );
    
    /* size */
    if (( pMobIndex->size = size_table.value(fread_word(fp)) ) == NO_FLAG)
        throw FileFormatException("load_mobile %d bad size", pMobIndex->vnum);

    pMobIndex->material                = str_dup(fread_word( fp ));

    for ( ; ; ) {
        char letter = fread_letter( fp );

        if (letter == 'F') {
            char *word;
            long vector;

            word = fread_word(fp);
            vector = fread_flag(fp);

            if (!str_prefix(word,"act"))
                REMOVE_BIT(pMobIndex->act,vector);
            else if (!str_prefix(word,"aff"))
                REMOVE_BIT(pMobIndex->affected_by,vector);
            else if (!str_prefix(word,"off"))
                REMOVE_BIT(pMobIndex->off_flags,vector);
            else if (!str_prefix(word,"imm"))
                REMOVE_BIT(pMobIndex->imm_flags,vector);
            else if (!str_prefix(word,"res"))
                REMOVE_BIT(pMobIndex->res_flags,vector);
            else if (!str_prefix(word,"vul"))
                REMOVE_BIT(pMobIndex->vuln_flags,vector);
            else if (!str_prefix(word,"for"))
                REMOVE_BIT(pMobIndex->form,vector);
            else if (!str_prefix(word,"par"))
                REMOVE_BIT(pMobIndex->parts,vector);
            else {
                throw FileFormatException("load_mobile %d flag remove: flag not found", pMobIndex->vnum);
            }
        } else if (letter == 'D') {
            long vector = fread_flag(fp);
            SET_BIT(pMobIndex->detection, vector);
        } else if (letter == 'P') {
            fread_number(fp);

            LogStream::sendWarning() << "skipping deprecated `init' field for mob vnum " << pMobIndex->vnum << endl;
        } else if (letter == 'T') {
            if (shopper || ( shopper = allocate_shop( ) ))
                shopper->load( fread_dlstring_to_eol( fp ) );
            else
                fread_to_eol( fp );
        } else if (letter == 'S') {
            pMobIndex->spec_fun.name = fread_word( fp );
            pMobIndex->spec_fun.func = spec_lookup( pMobIndex->spec_fun.name.c_str( ) );
            if(!pMobIndex->spec_fun.func) {
                LogStream::sendError() 
                    << "Mob: " << pMobIndex->vnum 
                    << ": special " << pMobIndex->spec_fun.name
                    << " not found." << endl;
            }
            if (pMobIndex->spec_fun.name == "spec_repairman") {
                if (shopper || ( shopper = allocate_shop( ) ))
                    shopper->load( pMobIndex->spec_fun.name );
            }

        } else if (letter == 'G') {
            pMobIndex->practicer.fromString( fread_dlstring( fp ) );
            // Note: religion and any other new things won't be saved to the old format.
        } else if (letter == 'N') {
            pMobIndex->gram_number = Grammar::Number(fread_word(fp));
        } else if (letter == 'X') {
            char *token = fread_word(fp);

            if (!str_cmp(token, "smell"))
                pMobIndex->smell = fread_dlstring(fp);
            else
                LogStream::sendError() << "Unknown mob token " << token << endl;
        } else {

            ungetc(letter,fp);
            break;
        }
    }

    /* read behavior description as an xml-document */
    MobileBehaviorManager::parse( pMobIndex, fp );
    
    if (shopper) 
        convert_shop( pMobIndex, shopper );
}

/*
 * Snarf a mob section.  new style
 */
void 
load_mobiles( FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    static MOB_INDEX_DATA zeroMobIndex;

    for ( ; ; ) {
        int vnum;
        char letter;
        int iHash;

        letter                          = fread_letter( fp );
        if ( letter != '#' ) 
            throw FileFormatException("load_mobiles: # not found");
        
        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        if (dup_mob_vnum( vnum )) 
            throw FileFormatException("load_mobiles: vnum %d duplicated", vnum);
        
        pMobIndex = new MOB_INDEX_DATA;
        *pMobIndex = zeroMobIndex;
        pMobIndex->vnum = vnum;
        pMobIndex->area        = area_last;
        load_mobile(fp, pMobIndex);
        newmobs++;
        
        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;

        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper( pMobIndex );

        top_mob_index++;

        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
                
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
    }

    return;
}

/*
 * parse one object. no explicit side effects
 */
void
load_object(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
    Affect *paf = NULL;
    char letter;

    pObjIndex->wrapper                = 0;
    pObjIndex->rnext                = 0;

    pObjIndex->new_format       = true;
    pObjIndex->reset_num        = 0;
    pObjIndex->name             = fread_string( fp );
    pObjIndex->short_descr      = fread_string( fp );
    pObjIndex->description      = fread_string( fp );
    pObjIndex->material                = fread_string( fp );

    pObjIndex->item_type        = item_table.value( fread_word( fp ) );

    pObjIndex->extra_flags      = fread_flag( fp );
    pObjIndex->wear_flags       = fread_flag( fp );

    switch(pObjIndex->item_type) {
    case ITEM_WEAPON:
        if (( pObjIndex->value[0] = weapon_class.value(fread_word(fp)) ) == NO_FLAG)
            throw FileFormatException( "load_object %d bad weapon type", pObjIndex->vnum );
        pObjIndex->value[1]        = fread_number(fp);
        pObjIndex->value[2]        = fread_number(fp);
        if (( pObjIndex->value[3]        = weapon_flags.value(fread_word(fp)) ) == NO_FLAG)
            throw FileFormatException( "load_object %d bad attack type", pObjIndex->vnum );
        pObjIndex->value[4]        = fread_flag(fp);
        break;
    case ITEM_CONTAINER:
        pObjIndex->value[0]        = fread_number(fp);
        pObjIndex->value[1]        = fread_flag(fp);
        pObjIndex->value[2]        = fread_number(fp);
        pObjIndex->value[3]        = fread_number(fp);
        pObjIndex->value[4]        = fread_number(fp);
        break;
    case ITEM_FOUNTAIN:
        pObjIndex->value[0]     = fread_number(fp);
        pObjIndex->value[1]     = fread_number(fp);
        pObjIndex->value[2]     = liquidManager->lookup(fread_word(fp));
        pObjIndex->value[3]     = fread_number(fp);
        pObjIndex->value[4]     = fread_number(fp);
        break;
    case ITEM_DRINK_CON:
        pObjIndex->value[0]     = fread_number(fp);
        pObjIndex->value[1]     = fread_number(fp);
        pObjIndex->value[2]     = liquidManager->lookup(fread_word(fp));
        pObjIndex->value[3]     = fread_flag(fp);
        pObjIndex->value[4]     = fread_number(fp);
        break;
    case ITEM_WAND:
    case ITEM_STAFF:
        pObjIndex->value[0]        = fread_number(fp);
        pObjIndex->value[1]        = fread_number(fp);
        pObjIndex->value[2]        = fread_number(fp);
        pObjIndex->value[3]        = SkillManager::getThis( )->lookup(fread_word(fp));
        pObjIndex->value[4]        = fread_number(fp);
        break;
    case ITEM_POTION:
    case ITEM_PILL:
    case ITEM_SCROLL:
        pObjIndex->value[0]        = fread_number(fp);
        pObjIndex->value[1]        = SkillManager::getThis( )->lookup(fread_word(fp));
        pObjIndex->value[2]        = SkillManager::getThis( )->lookup(fread_word(fp));
        pObjIndex->value[3]        = SkillManager::getThis( )->lookup(fread_word(fp));
        pObjIndex->value[4]        = SkillManager::getThis( )->lookup(fread_word(fp));
        break;
    default:
        pObjIndex->value[0]     = fread_flag( fp );
        pObjIndex->value[1]     = fread_flag( fp );
        pObjIndex->value[2]     = fread_flag( fp );
        pObjIndex->value[3]     = fread_flag( fp );
        pObjIndex->value[4]        = fread_flag( fp );
        break;
    }
    pObjIndex->level                = fread_number( fp );
    pObjIndex->weight           = fread_number( fp );
    pObjIndex->cost             = fread_number( fp );
    pObjIndex->behavior                = 0;
    pObjIndex->limit            = -1;

    /* condition */
    letter                         = fread_letter( fp );
    switch (letter) {
    case ('P') :                pObjIndex->condition = 100; break;
    case ('G') :                pObjIndex->condition =  90; break;
    case ('A') :                pObjIndex->condition =  75; break;
    case ('W') :                pObjIndex->condition =  50; break;
    case ('D') :                pObjIndex->condition =  25; break;
    case ('B') :                pObjIndex->condition =  10; break;
    case ('R') :                pObjIndex->condition =   0; break;
    default:                        pObjIndex->condition = 100; break;
    }

    for ( ; ; ) {
        char letter;

        letter = fread_letter( fp );

        if ( letter == 'A' ) {
            paf                     = dallocate( Affect );
            paf->where                    = TO_OBJECT;
            paf->type.assign( gsn_none );
            paf->level              = pObjIndex->level;
            paf->duration           = -1;
            paf->location           = fread_number( fp );
            paf->modifier           = fread_number( fp );
            paf->bitvector          = 0;

            pObjIndex->affected.push_front(paf);
            top_affect++;
        } else if (letter == 'F') {
            paf                     = dallocate( Affect );
            
            letter                     = fread_letter(fp);
            switch (letter) {
            case 'A': paf->where    = TO_AFFECTS;   break;
            case 'I': paf->where    = TO_IMMUNE;    break;
            case 'R': paf->where    = TO_RESIST;    break;
            case 'V': paf->where    = TO_VULN;            break;
            case 'D': paf->where    = TO_DETECTS;   break;
            default:
                throw FileFormatException("load_object %d bad 'where' on flag set", pObjIndex->vnum);
            }

            paf->type.assign( gsn_none );
            paf->level              = pObjIndex->level;
            paf->duration           = -1;
            paf->location           = fread_number(fp);
            paf->modifier           = fread_number(fp);
            paf->bitvector          = fread_flag(fp);

            pObjIndex->affected.push_front(paf);
            top_affect++;
        } else if ( letter == 'E' ) {
            EXTRA_DESCR_DATA *ed;

            ed                      = new_extra_descr();
            ed->keyword             = fread_string( fp );
            ed->description         = fread_string( fp );

            ed->next                = pObjIndex->extra_descr;
            pObjIndex->extra_descr  = ed;
            top_ed++;
        } else if (letter == 'P') {
            fread_number(fp);

            LogStream::sendWarning() << "skipping deprecated `init' field for obj vnum " << pObjIndex->vnum << endl;
        } else if (letter == 'L') {
            pObjIndex->limit = fread_number(fp);
            
        } else if (letter == 'G') {
            pObjIndex->gram_gender = Grammar::MultiGender(fread_word(fp));
        } else if (letter == 'X') {
            char *token = fread_word(fp);

            if (!str_cmp(token, "sound"))
                pObjIndex->sound = fread_dlstring(fp);
            else if (!str_cmp(token, "smell"))
                pObjIndex->smell = fread_dlstring(fp);
            else
                LogStream::sendError() << "Unknown object token " << token << endl;
        } else {
            ungetc( letter, fp );
            break;
        }
    }
    
    /* read behavior description as an xml-document */
    pObjIndex->behavior                = 0;
    ObjectBehaviorManager::parse( pObjIndex, fp );
}

/*
 * Snarf an obj section. new style
 */
void 
load_objects( FILE *fp ) 
{
    static OBJ_INDEX_DATA zeroObjIndex;
    OBJ_INDEX_DATA *pObjIndex = NULL;

    if ( area_last == 0 ) {
        LogStream::sendFatal( ) << "Load_resets: no #AREA seen yet." << endl;
        exit( 1 );
    }

    for( ; ; ) {
        int vnum = 0;
        char letter;
        int iHash;

        letter                          = fread_letter( fp );
        if ( letter != '#' ) 
            throw FileFormatException("load_objects: # not found");

        vnum                            = fread_number( fp );
        if ( vnum == 0 )
            break;

        if (dup_obj_vnum(vnum)) 
            throw FileFormatException("load_objects: vnum %d duplicated", vnum);
        
        pObjIndex = new OBJ_INDEX_DATA;
        *pObjIndex = zeroObjIndex;
        pObjIndex->vnum = vnum;
        pObjIndex->area        = area_last;
        load_object(fp, pObjIndex);

        newobjs++;
        
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        
        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper( pObjIndex );

        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;       /* OLC */
    }
}




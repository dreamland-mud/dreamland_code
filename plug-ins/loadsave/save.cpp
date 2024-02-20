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


#ifdef HAVE_CONFIG_H
        #include "config.h"
#endif

#include <algorithm>

#include <sys/types.h>
#include <ctype.h>
#include <cstdio>
#include <cstring>
#include <time.h>


#include "fileformatexception.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "skill.h"
#include "skillgroup.h"
#include "skillreference.h"

#include "mobilebehavior.h"
#include "mobilebehaviormanager.h"
#include "objectbehavior.h"
#include "objectbehaviormanager.h"

#include "feniamanager.h"

#include "pcharactermanager.h"
#include "pcharactermemory.h"
#include "affectmanager.h"
#include "objectmanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "affect.h"
#include "room.h"
#include "desire.h"
#include "liquid.h"

#include "dreamland.h"
#include "merc.h"

#include "act.h"
#include "save.h"
#include "loadsave.h"
#include "fread_utils.h"
#include "compatflags.h"
#include "itemevents.h"
#include "vnum.h"
#include "def.h"

GSN(none);
GSN(doppelganger);
GSN(dispel_affects);
GSN(dispel_magic);
GSN(spell_resistance);
GSN(magic_resistance);
GSN(enchant_weapon);
GSN(enchant_armor);
GSN(katana);
DESIRE(drunk);
DESIRE(full);
DESIRE(hunger);
DESIRE(thirst);
DESIRE(bloodlust);
WEARLOC(none);
CLAN(none);
void password_set( PCMemoryInterface *pci, const DLString &plainText );
const FlagTable * affect_where_to_table(int where);
int affect_table_to_where(const FlagTable *table, const GlobalRegistryBase *registry);

static DLString id_to_string(long long id)
{
    ostringstream os;
    os << id;
    return os.str();
}

static void convert_skill( int &sn )
{
    if (sn == gsn_dispel_magic)
        sn = gsn_dispel_affects;
    else if (sn == gsn_magic_resistance)
        sn = gsn_spell_resistance;
}

static const DLString &skillname(int index)
{
    Skill *skill = SkillManager::getThis( )->find(index);
    return skill ? skill->getName() : gsn_none->getName();
}

static void convert_obj_values( Object *obj )
{
    static const char * liquid_names [] = {
    "water", "beer", "red wine", "ale", "dark ale", "whisky", "lemonade",
    "firebreather", "local specialty", "slime mold juice",
    "milk", "tea", "coffee", "blood", "salt water", "coke", "root beer",
    "elvish wine", "white wine", "champagne", "mead", "rose wine",
    "benedictine wine", "vodka", "cranberry juice", "orange juice",
    "absinthe", "brandy", "aquavit", "schnapps", "icewine", "amontillado",
    "sherry", "framboise", "rum", "cordial", "valerian tincture",
    "chocolate", "grape juice", 
    };
    static const int liquid_count = 39;

    switch (obj->item_type) {
    case ITEM_FOUNTAIN:
    case ITEM_DRINK_CON:
        if (obj->value2() >= 0 && obj->value2() < liquid_count) 
            obj->value2(liquidManager->lookup( liquid_names[obj->value2()] ));

        break;

    default:
        break;
    }
}

static void convert_religion( PCharacter *pc, int number )
{
    static const char *relig_names [] =  { 
     "none",       "atum-ra",    "zeus",       "siebele",    "shamash",    
     "ahuramazda", "ehrumen",    "deimos",     "phobos",     "odin",       
     "teshub",     "ares",       "goktengri",  "hera",       "venus",      
     "seth",       "enki",       "eros", 
    };
    
    pc->setReligion( religionManager->find( relig_names[number] )->getName( ) );
}

Affect * fread_affect( FILE *fp ) 
{
    Affect *paf;
    int sn;
    char *word = fread_word( fp );
    DLString globalString;
    int where;

    sn = SkillManager::getThis( )->lookup( word );
    convert_skill( sn );

    paf = AffectManager::getThis()->getAffect();

    try {
        paf->type = sn;
        where        = fread_number(fp); 
        paf->level        = fread_number(fp);
        paf->duration        = fread_number(fp);
        paf->modifier        = fread_number(fp);
        paf->location.setTable(&apply_flags);
        paf->location        = fread_number(fp);
        paf->bitvector.setTable(affect_where_to_table(where));
        paf->bitvector.setValue(fread_number(fp));

        globalString    = fread_dlstring_to_eol(fp);
        globalString.substitute('\r', ' ').substitute('\n', ' ');
        globalString.stripWhiteSpace( );

        if (where == TO_LOCATIONS) {
            paf->global.setRegistry( wearlocationManager );
            paf->global.fromString( globalString );
        } else if (where == TO_LIQUIDS) {
            paf->global.setRegistry( liquidManager );
            paf->global.fromString( globalString );
        } else if (where == TO_SKILLS) {
            paf->global.setRegistry( skillManager );
            paf->global.fromString( globalString );
        } else if (where == TO_SKILL_GROUPS) {
            paf->global.setRegistry( skillGroupManager );
            paf->global.fromString( globalString );
        }

    } catch (const FileFormatException &e) {
        AffectManager::getThis()->extract(paf);
        throw e;
    }

    return paf;
}

void fwrite_affect( const char *label, FILE *fp, Affect *paf )
{
    if (paf->type == gsn_doppelganger)
        return;

    fprintf( fp, "%s '%s' %3d %3d %3d %3d %3d %10lld %s\n",
            label, 
            paf->type->getName( ).c_str( ),
            affect_table_to_where(paf->bitvector.getTable(), paf->global.getRegistry()),
            paf->level.getValue(),
            paf->duration.getValue(),
            paf->modifier.getValue(),
            paf->location.getValue(),
            paf->bitvector.getValue(),
            paf->global.toString( ).c_str( ));
}

char *print_flags(int flag)
{
    int count, pos = 0;
    static char buf[52];


    for (count = 0; count < 32;  count++)
    {
        if (IS_SET(flag,1<<count))
        {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0)
    {
        buf[pos] = '0';
        pos++;
    }

    buf[pos] = '\0';

    return buf;
}


/*
 * Array of containers read for proper re-nesting of objects.
 */
Object *        rgObjNest        [MAX_NEST];




/*
 * Write the char.
 */
void fwrite_char( PCharacter *ch, FILE *fp )
{

        fprintf( fp, "#%s\n",  "PLAYER"        );
        for (auto &paf: ch->affected)
            fwrite_affect( "Affc", fp, paf );

        fprintf( fp, "End\n\n" );
}

/* write a pet */
void fwrite_pet( NPCharacter *pet, FILE *fp)
{
        fprintf(fp,"#PET\n");

        fprintf(fp,"Vnum %d\n",pet->pIndexData->vnum);

        fprintf(fp,"Name %s~\n", pet->getNameC() );

        if (pet->in_room && pet->master && pet->master->in_room != pet->in_room)
            fprintf(fp,"Room %d\n", pet->in_room->vnum);
            
        fprintf( fp, "Id   %s\n", id_to_string(pet->getID()).c_str() );
        if (pet->getRealShortDescr( ))
                fprintf(fp,"ShD  %s~\n", pet->getShortDescr( ));
        if (pet->getRealLongDescr( ))
                fprintf(fp,"LnD  %s~\n", pet->getLongDescr( ));
        if (pet->getRealDescription( ))
                fprintf(fp,"Desc %s~\n", pet->getDescription( ));

        if (pet->getRace( )->getName( ) != pet->pIndexData->race) 
                fprintf(fp,"Race %s~\n", pet->getRace( )->getName( ).c_str( ));

        fprintf(fp,"Sex  %d\n", pet->getSex( ) );

        if (pet->getRealLevel( ) != pet->pIndexData->level)
                fprintf(fp,"Levl %d\n", pet->getRealLevel( ) );

        fprintf(fp, "HMV  %d %d %d %d %d %d\n",
                pet->hit.getValue( ), pet->max_hit.getValue( ), pet->mana.getValue( ), pet->max_mana.getValue( ), pet->move.getValue( ), pet->max_move.getValue( ));

        if (pet->gold > 0)
                fprintf(fp,"Gold %d\n",pet->gold.getValue( ));

        if (pet->silver > 0)
                fprintf(fp,"Silv %d\n",pet->silver.getValue( ));

        if (pet->exp > 0)
                fprintf(fp, "Exp  %d\n", pet->exp.getValue( ));

        if (pet->act != pet->pIndexData->act)
                fprintf(fp, "Act  %s\n", print_flags(pet->act));

        if (pet->affected_by != pet->pIndexData->affected_by)
                fprintf(fp, "AfBy %s\n", print_flags(pet->affected_by));

        if (pet->detection != pet->pIndexData->detection)
                fprintf(fp, "Detect %s\n", print_flags(pet->detection));

        if (pet->comm != 0)
                fprintf(fp, "Comm %s\n", print_flags(pet->comm));

        fprintf(fp,"Pos  %d\n", pet->position == POS_FIGHTING ? POS_STANDING : pet->position.getValue( ));

        if (pet->saving_throw != 0)
                fprintf(fp, "Save %d\n", pet->saving_throw.getValue( ));

        if (pet->alignment != pet->pIndexData->alignment)
                fprintf(fp, "Alig %d\n", pet->alignment.getValue( ));

        if (pet->hitroll != pet->pIndexData->hitroll)
                fprintf(fp, "Hit  %d\n", pet->hitroll.getValue( ));

        if (pet->damroll != pet->pIndexData->damage[DICE_BONUS])
                fprintf(fp, "Dam  %d\n", pet->damroll.getValue( ));

        if (pet->damage[DICE_NUMBER] != pet->pIndexData->damage[DICE_NUMBER])
                fprintf(fp, "DamN  %d\n", pet->damage[DICE_NUMBER]);

        if (pet->damage[DICE_TYPE] != pet->pIndexData->damage[DICE_TYPE])
                fprintf(fp, "DamT  %d\n", pet->damage[DICE_TYPE]);

        fprintf(fp, "ACs  %d %d %d %d\n",
                pet->armor[0],pet->armor[1],pet->armor[2],pet->armor[3]);

        fprintf(fp, "Attr %d %d %d %d %d %d\n",
                pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
                pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
                pet->perm_stat[STAT_CON], pet->perm_stat[STAT_CHA]);

        fprintf(fp, "AMod %d %d %d %d %d %d\n",
                pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
                pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
                pet->mod_stat[STAT_CON], pet->mod_stat[STAT_CHA]);

        for (auto &paf: pet->affected)
            fwrite_affect( "Affc", fp, paf );

        fprintf(fp,"End\n");
        return;
}


/* write a mobile */
void fwrite_mob( NPCharacter *mob, FILE *fp)
{
        if ( mob->in_room == 0 )
        {
                bug( "Write_mobile: mobile not in room! ", 0 );
                return;
        }

        if (IS_SET(mob->pIndexData->area->area_flag, AREA_NOSAVEDROP))
            return;

        if (IS_SET(mob->act, ACT_NOSAVEDROP))
            return;

        fprintf(fp,"#MOBILE\n");

        fprintf(fp,"Vnum %d\n",mob->pIndexData->vnum);

        fprintf(fp,"Name %s~\n", mob->getNameC() );

        fprintf( fp, "Id   %s\n", id_to_string(mob->getID()).c_str() );
        
        fprintf( fp, "Room %d\n", mob->in_room->vnum );

        if ( mob->zone )
                fprintf( fp, "RZone %s~\n", mob->zone->area_file->file_name );
        if (mob->reset_room != 0)
            fprintf(fp, "RRoom %d\n", mob->reset_room); 

        if (mob->getRealShortDescr( ))
            fprintf(fp,"ShD  %s~\n", mob->getShortDescr( ));
        if (mob->getRealLongDescr( ))
            fprintf(fp,"LnD  %s~\n", mob->getLongDescr( ));
        if (mob->getRealDescription( ))
            fprintf(fp,"Desc %s~\n", mob->getDescription( ));

        if (mob->getRace( )->getName( ) != mob->pIndexData->race) 
                fprintf(fp,"Race %s~\n", mob->getRace( )->getName( ).c_str( ));

        fprintf(fp,"Sex  %d\n", mob->getSex( ) );

        if (mob->getRealLevel( ) != mob->pIndexData->level)
                fprintf(fp,"Levl %d\n", mob->getRealLevel( ) );

        if (mob->getClan() != clan_none)
            fprintf(fp, "Clan %s~\n", mob->getClan().getName().c_str());

        fprintf(fp, "HMV  %d %d %d %d %d %d\n",
                mob->hit.getValue( ), mob->max_hit.getValue( ), mob->mana.getValue( ), mob->max_mana.getValue( ), mob->move.getValue( ), mob->max_move.getValue( ));

        if (mob->gold > 0)
                fprintf(fp,"Gold %d\n",mob->gold.getValue( ));

        if (mob->silver > 0)
                fprintf(fp,"Silv %d\n",mob->silver.getValue( ));

        fprintf(fp, "Wearloc %s~\n", mob->wearloc.toString().c_str());

        if (mob->exp > 0)
                fprintf(fp, "Exp  %d\n", mob->exp.getValue( ));

        if (mob->timer != 0)
                fprintf( fp, "Timer %d\n", mob->timer );

        if (mob->act != mob->pIndexData->act)
                fprintf(fp, "Act  %s\n", print_flags(mob->act));

        if (mob->affected_by != mob->pIndexData->affected_by)
                fprintf(fp, "AfBy %s\n", print_flags(mob->affected_by & ~AFF_CHARM));

        if (mob->detection != mob->pIndexData->detection)
                fprintf(fp, "Detect %s\n", print_flags(mob->detection));

        if (mob->comm != 0)
                fprintf(fp, "Comm %s\n", print_flags(mob->comm));

        fprintf(fp,"Pos  %d\n", mob->position == POS_FIGHTING ? POS_STANDING : mob->position.getValue( ));

        if (mob->saving_throw != 0)
                fprintf(fp, "Save %d\n", mob->saving_throw.getValue( ));

        if (mob->alignment != mob->pIndexData->alignment)
                fprintf(fp, "Alig %d\n", mob->alignment.getValue( ));

        if (mob->hitroll != mob->pIndexData->hitroll)
                fprintf(fp, "Hit  %d\n", mob->hitroll.getValue( ));

        if (mob->damroll != mob->pIndexData->damage[DICE_BONUS])
                fprintf(fp, "Dam  %d\n", mob->damroll.getValue( ));

        if (mob->damage[DICE_NUMBER] != mob->pIndexData->damage[DICE_NUMBER])
                fprintf(fp, "DamN  %d\n", mob->damage[DICE_NUMBER]);

        if (mob->damage[DICE_TYPE] != mob->pIndexData->damage[DICE_TYPE])
                fprintf(fp, "DamT  %d\n", mob->damage[DICE_TYPE]);

        fprintf(fp, "ACs  %d %d %d %d\n",
                mob->armor[0],mob->armor[1],mob->armor[2],mob->armor[3]);

        fprintf(fp, "Attr %d %d %d %d %d %d\n",
                mob->perm_stat[STAT_STR], mob->perm_stat[STAT_INT],
                mob->perm_stat[STAT_WIS], mob->perm_stat[STAT_DEX],
                mob->perm_stat[STAT_CON], mob->perm_stat[STAT_CHA]);

        fprintf(fp, "AMod %d %d %d %d %d %d\n",
                mob->mod_stat[STAT_STR], mob->mod_stat[STAT_INT],
                mob->mod_stat[STAT_WIS], mob->mod_stat[STAT_DEX],
                mob->mod_stat[STAT_CON], mob->mod_stat[STAT_CHA]);

        for (auto &paf: mob->affected)
            fwrite_affect( "Affc", fp, paf );

        fprintf(fp,"End\n");
        
        MobileBehaviorManager::save( mob, fp );

        if ( mob->carrying != 0 )
        {
                fprintf( fp, "\n" );
                fwrite_obj( mob, mob->carrying, fp, 0 );
        }

        return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( Character *ch, Object *obj, FILE *fp, int iNest )
{
        Object *last = 0;
        Object *curr, *prelast;

        if ( obj == 0 )
                return;

        last = obj;

        while ( last->next_content != 0 )
                last = last->next_content;

        while ( last != 0 )
        {
                curr = obj;

                prelast = 0;
                
                while ( curr != last )
                {
                        prelast = curr;
                        curr = curr->next_content;
                }

                last = prelast;

                fwrite_obj_0( ch, curr, fp, iNest );
        }
}


void fwrite_obj_0( Character *ch, Object *obj, FILE *fp, int iNest )
{
        EXTRA_DESCR_DATA *ed;

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
//        if ( obj->next_content != 0 )
//                fwrite_obj( ch, obj->next_content, fp, iNest );


    if (IS_SET(obj->pIndexData->area->area_flag, AREA_NOSAVEDROP))
        return;

    if (IS_SET(obj->extra_flags, ITEM_NOSAVEDROP))
        return;

    /*
     * Castrate storage characters and rooms.
     */
        if (ch != 0 && !ch->is_npc( ) && !ch->is_immortal( ))
        {
            if (!obj->hasOwner( ch ) && obj->mustDisappear( ch ))
            {
                oldact("$o1 рассыпается трухой!",ch,obj,0,TO_CHAR);
                extract_obj( obj );
                return;
            }

            if (obj->behavior)
                if (obj->behavior->save( ))
                    return;
        }



        try
        {
                fprintf( fp, "#O\n" );
                fprintf( fp, "Vnum %d\n",           obj->pIndexData->vnum                );
                
                if (obj->timestamp > 0) {
                    fprintf( fp, "TS %s\n", id_to_string(obj->timestamp).c_str() );
                }

                fprintf( fp, "Id   %s\n", id_to_string(obj->getID()).c_str() );

                if (obj->reset_room != 0)
                    fprintf(fp, "RRoom %d\n", obj->reset_room);
                if (obj->reset_mob != 0)
                    fprintf(fp, "RMob %s\n", id_to_string(obj->reset_mob).c_str());
                if (obj->reset_obj != 0)
                    fprintf(fp, "RObj %s\n", id_to_string(obj->reset_obj).c_str());

                fprintf( fp, "Cond %d\n",                obj->condition                        );

                fprintf( fp, "Nest %d\n",        iNest                       );

                if (!obj->pocket.empty( ))
                    fprintf( fp, "Pocket %s~\n", obj->pocket.c_str( ) );

                /* these data are only used if they do not match the defaults */
                if (obj->getRealMaterial( ))
                    fprintf( fp, "Material %s~\n",  obj->getMaterial( ));
                if (obj->getRealName( ))
                    fprintf( fp, "Name %s~\n",        obj->getName( )            );
                if (obj->getRealShortDescr( ))
                    fprintf( fp, "ShD  %s~\n",        obj->getShortDescr( ) );
                if (obj->getRealDescription( ))
                    fprintf( fp, "Desc %s~\n",        obj->getDescription( )  );
                if (!obj->getOwner().empty())
                    fprintf( fp, "Ownr %s~\n",        obj->getOwner().c_str());

                if ( obj->extra_flags != obj->pIndexData->extra_flags)
                        fprintf( fp, "ExtF %d\n",        obj->extra_flags             );
                if ( obj->wear_flags != obj->pIndexData->wear_flags)
                        fprintf( fp, "WeaF %d\n",        obj->wear_flags                     );
                if ( obj->item_type != obj->pIndexData->item_type)
                        fprintf( fp, "Ityp %d\n",        obj->item_type                     );
                if ( obj->weight != obj->pIndexData->weight)
                        fprintf( fp, "Wt   %d\n",        obj->weight                     );

                        /* variable data */

                fprintf( fp, "Wearloc %s\n",   obj->wear_loc.getName( ).c_str( ) );
                if (obj->level != obj->pIndexData->level)
                        fprintf( fp, "Lev  %d\n",        obj->level                     );
                if (obj->timer != 0)
                        fprintf( fp, "Time %d\n",        obj->timer             );
                fprintf( fp, "Cost %d\n",        obj->cost                     );
                if ( obj->value0() != obj->pIndexData->value[0]
                        || obj->value1() != obj->pIndexData->value[1]
                        || obj->value2() != obj->pIndexData->value[2]
                        || obj->value3() != obj->pIndexData->value[3]
                        || obj->value4() != obj->pIndexData->value[4] )
                        fprintf( fp, "Val  %d %d %d %d %d\n",
                                obj->value0(), obj->value1(), obj->value2(), obj->value3(),
                                obj->value4() );

                switch ( obj->item_type )
                {
                case ITEM_POTION:
                case ITEM_SCROLL:
                        if ( obj->value1() > 0 )
                                fprintf( fp, "Spell 1 '%s'\n", skillname(obj->value1()).c_str() );
                        if ( obj->value2() > 0 )
                                fprintf( fp, "Spell 2 '%s'\n", skillname(obj->value2()).c_str() );
                        if ( obj->value3() > 0 )
                                fprintf( fp, "Spell 3 '%s'\n", skillname(obj->value3()).c_str() );
                        if ( obj->value4() > 0 )
                                fprintf( fp, "Spell 4 '%s'\n", skillname(obj->value4()).c_str() );
                        break;

                case ITEM_PILL:
                case ITEM_STAFF:
                case ITEM_WAND:
                        if ( obj->value3() > 0 )
                                fprintf( fp, "Spell 3 '%s'\n", skillname(obj->value3()).c_str() );
                        break;

                case ITEM_FOUNTAIN:
                case ITEM_DRINK_CON:
                        fprintf( fp, "Liquid '%s'\n",  
                                     liquidManager->find( obj->value2() )->getName( ).c_str( ) );
                        break;
                }

                for (auto &paf: obj->affected)
                    fwrite_affect( "Affect", fp, paf );

                for ( ed = obj->extra_descr; ed != 0; ed = ed->next )
                {
                        fprintf( fp, "ExDe %s~ %s~\n", ed->keyword, ed->description );
                }

                for (Properties::const_iterator p = obj->properties.begin(); p != obj->properties.end(); p++)
                    fprintf(fp, "X %s %s~\n", p->first.c_str(), p->second.c_str());

                if (obj->gram_gender != Grammar::MultiGender::UNDEF)
                        fprintf(fp, "Gender %s\n", obj->gram_gender.toString());

                fprintf( fp, "End\n" );
                ObjectBehaviorManager::save( obj, fp );
                fprintf( fp, "\n" );

                if ( obj->contains != 0 )
                        fwrite_obj( ch, obj->contains, fp, iNest + 1 );
        }
        catch(const Exception &e)
        {
                if ( obj != 0 && obj->pIndexData != 0 )
                {
                        bug("{RSave_object: filling {Cvnum %d %s{R FAILED!!!!!!!!", obj->pIndexData->vnum, obj->getName( ));
                }
                else if ( obj !=0 && obj->getName( ) != 0 )
                {
                       bug("{RSave_object: filling {C%s{R FAILED!!!!!!!!", obj->getName( ) );
                }
                else
                {
                        bug("{RSave_object: filling{R FAILED!!!!!!!!" );
                }
        }

        return;
}




/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                        \
                                if ( !str_cmp( word, literal ) )        \
                                {                                        \
                                    field  = value;                        \
                                    fMatch = true;                        \
                                    break;                                \
                                }

#define KEYSKIP( literal )                                                \
                                if ( !str_cmp( word, literal ) )        \
                                {                                        \
                                    fread_to_eol( fp );                        \
                                    fMatch = true;                        \
                                    break;                                \
                                }
#define KEYV( literal, field, value )                                        \
                                if ( !str_cmp( word, literal ) )        \
                                {                                        \
                                    field.setValue(value);              \
                                    fMatch = true;                        \
                                    break;                                \
                                }

void fread_char_raw( PCharacter *ch, FILE *fp )
{
    const char *word="End";
    bool fMatch = true;
    int dummy;

    LogStream::sendNotice( ) << "Loading " << ch->getName( ) << '.' << endl;

    for ( ; ; )
    {
        word   = feof( fp ) ? "End" : fread_word( fp );
        fMatch = false;

        switch ( dl_toupper(word[0]) )
        {
        case '*':
            fMatch = true;
            fread_to_eol( fp );
            break;

        case 'A':
            KEY( "Act",                ch->act,                fread_flag( fp ) );
            KEY( "AffectedBy",        ch->affected_by,        fread_flag( fp ) );
            KEY( "AfBy",        ch->affected_by,        fread_flag( fp ) );
            KEY( "Alignment",        ch->alignment,                fread_number( fp ) );
            KEY( "Alig",        ch->alignment,                fread_number( fp ) );
            KEY( "AntKilled",        ch->anti_killed,fread_number( fp ) );

            if (!str_cmp(word,"ACs"))
            {
                int i;

                for (i = 0; i < 4; i++)
                    ch->armor[i] = fread_number(fp);
                fMatch = true;
                break;
            }

            if (!str_cmp(word, "Affc"))
            {
                Affect *paf = fread_affect( fp );
                ch->affected.push_front(paf);
                fMatch = true;
                break;
            }

            if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod"))
            {
                int stat;
                for (stat = 0; stat < stat_table.size; stat ++)
                   ch->mod_stat[stat] = fread_number(fp);
                fMatch = true;
                break;
            }

            if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr"))
            {
                int stat;

                for (stat = 0; stat < stat_table.size; stat++)
                    ch->perm_stat[stat] = fread_number(fp);
                fMatch = true;
                break;
            }
            break;

        case 'B':
            KEY( "Bamfin",        ch->bamfin,        fread_string( fp ) );
            KEY( "Banks",       ch->bank_s,     fread_number( fp ) );
            KEY( "Bankg",       ch->bank_g,     fread_number( fp ) );
            KEY( "Bamfout",        ch->bamfout,        fread_string( fp ) );
            KEY( "Bin",                ch->bamfin,        fread_string( fp ) );
            KEY( "Bout",        ch->bamfout,        fread_string( fp ) );
            KEY( "Bless",        dummy,        fread_number( fp ) );

            if(!str_cmp(word, "BatlePrompt") || !str_cmp(word, "BatleProm")) {
                ch->batle_prompt = fread_string(fp);
                fMatch = true;
                break;
            }
            break;

        case 'C':
            KEYV( "Config",        ch->config,                fread_number( fp ) );

            if (!str_cmp(word,"CndC"))
            {
                ch->desires[desire_drunk] = fread_number( fp );
                ch->desires[desire_full] = fread_number( fp );
                ch->desires[desire_thirst] = fread_number( fp );
                ch->desires[desire_hunger] = fread_number( fp );
                ch->desires[desire_bloodlust] = fread_number( fp );
                fread_number( fp );
                fMatch = true;
                break;
            }
            KEY( "Comm",        ch->comm,                fread_flag( fp ) );
            KEY( "Comm_Add",        ch->add_comm,           fread_flag( fp ) );
            KEY( "Curse",        dummy,        fread_number( fp ) );

            break;

        case 'D':
            if( !str_cmp( word, "DeathT" ) ) {
              ch->last_death_time = fread_number( fp );
              fMatch = true;
              break;
            }
            KEY( "Damroll",        ch->damroll,                fread_number( fp ) );
            KEY( "Dam",                ch->damroll,                fread_number( fp ) );
            KEY( "Dead",        ch->death,        fread_number( fp ) );
                KEY( "Detect",        ch->detection,                fread_flag(fp)     );
            
            if (!str_cmp( word, "Description" ) || !str_cmp( word, "Desc" )) {
                char *word = fread_string( fp );
                ch->setDescription( word );
                free_string( word );
                fMatch = true;
            }
            break;

        case 'E':
            if ( !str_cmp( word, "End" ) )
            {
                return;
            }
            KEY( "Exp",                ch->exp,                fread_number( fp ) );
            KEYV( "Etho",        ch->ethos,                fread_number( fp ) );
            break;

        case 'G':
            KEY( "GHOST",        ch->ghost_time,        fread_number( fp ) );
            KEY( "Gold",        ch->gold,                fread_number( fp ) );
            break;

        case 'H':
            KEY( "Hitroll",        ch->hitroll,                fread_number( fp ) );
            KEY( "Hit",                ch->hitroll,                fread_number( fp ) );
            KEY( "Haskilled",        ch->has_killed, fread_number( fp ) );
            if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV"))
            {
                ch->hit                = fread_number( fp );
                ch->max_hit        = fread_number( fp );
                ch->mana        = fread_number( fp );
                ch->max_mana        = fread_number( fp );
                ch->move        = fread_number( fp );
                ch->max_move        = fread_number( fp );
                fMatch = true;
                break;
            }

            if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP"))
            {
                ch->perm_hit        = fread_number( fp );
                ch->perm_mana   = fread_number( fp );
                ch->perm_move   = fread_number( fp );
                fMatch = true;
                break;
            }

            break;

        case 'I':
            KEY( "InvisLevel",        ch->invis_level,        fread_number( fp ) );
            KEY( "Inco",        ch->incog_level,        fread_number( fp ) );
            KEY( "Invi",        ch->invis_level,        fread_number( fp ) );
            
            if (!str_cmp( word, "Id" )) {
                ch->setID( fread_number64( fp ) );
                fMatch = true;
                break;
            }

            break;

        case 'L':
            KEY( "LastLevel",        ch->last_level, fread_number( fp ) );
            KEY( "LLev",        ch->last_level, fread_number( fp ) );
            KEYV( "LogO",        ch->last_logoff, fread_number( fp ) );
            KEYSKIP( "LastTime" );
            break;
        case 'M':
                KEYSKIP( "MaxSkillPoints" );
                break;
        case 'N':
            if ( !str_cmp( word, "Name" ) )
            {
                char *nm = fread_string( fp );
                DLString name( nm );
                free_string(nm);
                ch->setName( name );
                fMatch = true;
                break;
            }
            break;

        case 'P':
            KEY( "PCethos",        ch->loyalty,        fread_number( fp ) );
            KEY( "PKFlag",        ch->PK_flag,        fread_number( fp ) );
            KEY( "PKTimeV",        ch->PK_time_v,        fread_number( fp ) );
            KEY( "PKTimeSK",        ch->PK_time_sk,        fread_number( fp ) );
            KEY( "PKTimeT",        ch->PK_time_t,        fread_number( fp ) );
            KEYV( "Position",        ch->position,                fread_number( fp ) );
            KEYV( "Pos",        ch->position,                fread_number( fp ) );
            KEY( "Practice",        ch->practice,                fread_number( fp ) );
            KEY( "Prac",        ch->practice,                fread_number( fp ) );
            
            if (!str_cmp( word, "Played" ) || !str_cmp( word, "Plyd" )) {
                ch->age.setTruePlayed( fread_number( fp ) );
                fMatch = true;
                break;
            }
            if (!str_cmp( word, "Pass" )) {
                char *pwd = fread_string( fp );
                password_set( ch, pwd );
                free_string( pwd );
                fMatch = true;
                break;
            }
            
            if(!str_cmp(word, "Prompt") || !str_cmp(word, "Prom")) {
                ch->prompt = fread_string(fp);
                fMatch = true;
                break;
            }
            break;
        case 'Q':
            if (!str_cmp(word, "QuestPnts")) {
                ch->setQuestPoints(fread_number(fp));
                fMatch = true;
                break;
            }
            break;

        case 'R':
            if ( !str_cmp( word, "Room" ) )
            {
                ch->start_room = fread_number( fp );
                fMatch = true;
                break;
            }
            if (!strcmp( word, "Relig" )) {
                convert_religion( ch, fread_number( fp ) );
                fMatch = true;
                break;
            }

            break;

        case 'S':
            KEY( "SavingThrow",        ch->saving_throw,        fread_number( fp ) );
            KEY( "Save",        ch->saving_throw,        fread_number( fp ) );
            KEY( "Scro",        ch->lines,                fread_number( fp ) );
            KEY( "Shadow",        ch->shadow,                fread_number( fp ) );
            KEY( "Silv",        ch->silver,             fread_number( fp ) );


            if ( !str_cmp( word, "Skill1" ) )
            {
                    int sn;
                    int value;
                    int timer;
                    char *temp;

                    value = fread_number( fp );
                    timer = fread_number( fp );
                    temp = fread_word( fp ) ;
                    /* forget */ fread_number( fp );
                    sn = SkillManager::getThis( )->lookup(temp);
                    convert_skill( sn );

                    if ( sn < 0 )
                    {
                        LogStream::sendWarning( ) << "Fread_char: unknown skill " << temp << endl;
                    }
                    else
                    {
                        PCSkillData &sk = ch->getSkillData( sn );
                        
                        sk.learned = value;
                        sk.timer = timer;
                    }
                    fMatch = true;
                    break;
            }

            break;

        case 'T':
            KEY( "Trai",        ch->train,        fread_number( fp ) );
            KEYSKIP( "Trust" );
            KEYSKIP( "Tru" );

            if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl"))
            {
                char *word = fread_string( fp );
                ch->setTitle( word );
                free_string( word );
                fMatch = true;
                break;
            }
            else if (!str_cmp( word, "TwitName" ) ) {
                fMatch = true;
                break;
            }

            break;

        case 'V':
            KEYSKIP( "Version" );
            KEYSKIP( "Vers" );
            break;

        case 'W':
            KEY( "Wimpy",        ch->wimpy,                fread_number( fp ) );
            KEY( "Wimp",        ch->wimpy,                fread_number( fp ) );
            KEY( "Wizn",        ch->wiznet,        fread_flag( fp ) );
            break;
        }

        if ( !fMatch )
        {
            LogStream::sendWarning( ) << "Fread_char: no match[" << word << "]." << endl;
            fread_to_eol( fp );
        }
    }
}

void fread_char( PCharacter *ch, FILE *fp )
{
    int percent;

    fread_char_raw( ch, fp );

    /* adjust hp mana move up  -- here for speed's sake */
    percent = ( dreamland->getCurrentTime( ) - ch->last_logoff) * 25 / ( 2 * 60 * 60);
    percent = std::min(percent,100);

    if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON) &&  !IS_AFFECTED(ch,AFF_PLAGUE))
    {
        ch->hit        += (ch->max_hit - ch->hit) * percent / 100;
        ch->mana    += (ch->max_mana - ch->mana) * percent / 100;
        ch->move    += (ch->max_move - ch->move)* percent / 100;
    }

    if (FeniaManager::wrapperManager)
        FeniaManager::wrapperManager->linkWrapper( ch );
}

/* load a pet from the forgotten reaches */
void fread_pet( PCharacter *ch, FILE *fp )
{
    const char *word;
    NPCharacter *pet;
    bool fMatch;
    int percent;
    bitstring_t create_flags = FCREATE_NOAFFECTS | FCREATE_NOCOUNT;

    /* first entry had BETTER be the vnum or we barf */
    word = feof(fp) ? "End" : fread_word(fp);
    if (!str_cmp(word,"Vnum"))
    {
            int vnum;

            vnum = fread_number(fp);
            if (get_mob_index(vnum) == 0)
        {
                bug("Fread_pet: bad vnum %d.",vnum);
            pet = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
        }
            else
                pet = create_mobile_org(get_mob_index(vnum), create_flags);
    }
    else
    {
        bug("Fread_pet: no vnum in file.",0);
        pet = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
    }

    for ( ; ; )
    {
            word         = feof(fp) ? "End" : fread_word(fp);
            fMatch = false;

            switch (dl_toupper(word[0]))
            {
            case '*':
                fMatch = true;
                fread_to_eol(fp);
                break;

            case 'A':
                KEY( "Act",                pet->act,                fread_flag(fp));
                KEY( "AfBy",        pet->affected_by,        fread_flag(fp));
                KEY( "Alig",        pet->alignment,                fread_number(fp));

                if (!str_cmp(word,"ACs"))
                {
                        int i;

                        for (i = 0; i < 4; i++)
                            pet->armor[i] = fread_number(fp);
                        fMatch = true;
                        break;
                }

            if (!str_cmp(word,"Affc"))
            {
                Affect *paf = fread_affect( fp );
                
                pet->affected.push_front(paf);
                fMatch          = true;
                break;
            }

                if (!str_cmp(word,"AMod"))
                {
                         int stat;

                         for (stat = 0; stat < stat_table.size; stat++)
                             pet->mod_stat[stat] = fread_number(fp);
                         fMatch = true;
                         break;
                }

                if (!str_cmp(word,"Attr"))
                {
                     int stat;

                     for (stat = 0; stat < stat_table.size; stat++)
                         pet->perm_stat[stat] = fread_number(fp);
                     fMatch = true;
                     break;
                }
                break;

             case 'C':
                 KEY( "Comm",        pet->comm,                fread_flag(fp));

                if (!str_cmp(word, "Clan")) {
                    char *word = fread_string(fp);
                    Clan *clan = clanManager->findExisting(word);
                    if (clan)
                        pet->setClan(clan->getName());

                    fMatch = true;
                    free_string( word );
                    break;
                }

                 break;

             case 'D':
                 KEY( "Dam",        pet->damroll,                fread_number(fp));
                 KEY( "DamT",       pet->damage[DICE_TYPE],      fread_number(fp));
                 KEY( "DamN",       pet->damage[DICE_NUMBER],    fread_number(fp));
                 KEY( "Detect",        pet->detection,                fread_flag(fp));

                if (!str_cmp( word, "Desc" )) {
                    char *word = fread_string( fp );
                    pet->setDescription( word );
                    free_string( word );
                    fMatch = true;
                    break;
                }
                 break;

             case 'E':
                 if (!str_cmp(word, "End")) {
                     pet->leader = ch;
                     pet->master = ch;
                     ch->pet = pet;
                     /* adjust hp mana move up  -- here for speed's sake */
                     percent = (dreamland->getCurrentTime() - ch->last_logoff) * 25 / (2 * 60 * 60);

                     if (percent > 0 && !IS_AFFECTED(ch, AFF_POISON) && !IS_AFFECTED(ch, AFF_PLAGUE)) {
                         percent = std::min(percent, 100);
                         pet->hit += (pet->max_hit - pet->hit) * percent / 100;
                         pet->mana += (pet->max_mana - pet->mana) * percent / 100;
                         pet->move += (pet->max_move - pet->move) * percent / 100;
                     }

                     if (FeniaManager::wrapperManager)
                         FeniaManager::wrapperManager->linkWrapper(pet);

                     LogStream::sendNotice() << "LOAD " << ch->getName() << " pet " << pet->getID() << " [" << pet->pIndexData->vnum << "]" << endl;    
                     return;
                 }
                 KEY("Exp", pet->exp, fread_number(fp));
                 break;

             case 'G':
                 KEY( "Gold",        pet->gold,                fread_number(fp));
                 break;
            
             case 'H':
                 KEY( "Hit",        pet->hitroll,                fread_number(fp));

                 if (!str_cmp(word,"HMV"))
                 {
                         pet->hit        = fread_number(fp);
                         pet->max_hit        = fread_number(fp);
                         pet->mana        = fread_number(fp);
                         pet->max_mana        = fread_number(fp);
                         pet->move        = fread_number(fp);
                         pet->max_move        = fread_number(fp);
                         fMatch = true;
                         break;
                 }
                 break;

        case 'I':
            if (!str_cmp( word, "Id" )) {
                pet->setID( fread_number64( fp ) );
                fMatch = true;
                break;
            }

            break;

             case 'L':
             KEYSKIP( "LogO" );
            if( !str_cmp( word, "Levl" ) )
            {
                    pet->setLevel( fread_number( fp ) );
                    fMatch = true;
                    break;
            }
            if (!str_cmp( word, "LnD" )) {
                char *word = fread_string( fp );
                pet->setLongDescr( word );
                fMatch = true;
                free_string( word );
                break;
            }
                break;

            case 'N':
                                if ( !str_cmp( word, "Name" ) )
                                {
                                    char *nm = fread_string( fp );
                                    DLString name( nm );
                                    free_string(nm);
                                    pet->setName( name );
                                    fMatch = true;
                                    break;
                                }
                 break;

            case 'P':
                 KEYV( "Pos",        pet->position,                fread_number(fp));
                 break;

        case 'R':
            if ( !str_cmp( word, "Race" ) )
            {
                    char *rnm = fread_string(fp);
                    pet->setRace( rnm );
                    free_string(rnm);
                    fMatch = true;
                break;
            } else if (!str_cmp( word, "Room" )) {
                    int rvnum = fread_number( fp );
                    pet->in_room = get_room_instance( rvnum );
                    if (!pet->in_room) {
                        LogStream::sendError( ) << "fread_pet: invalid room " << rvnum << " for " << ch->getName( ) << endl;
                    } 
                    fMatch = true;
                    break;
            }
                break;

            case 'S' :
                KEY( "Save",        pet->saving_throw,        fread_number(fp));
            KEY( "Silv",        pet->silver,            fread_number( fp ) );

            if (!str_cmp( word, "ShD" )) {
                char *word = fread_string( fp );
                pet->setShortDescr( word );
                fMatch = true;
                free_string( word );
                break;
            }
            if( !str_cmp( word, "Sex" ) )
            {
                pet->setSex( fread_number( fp ) );
                fMatch = true;
                break;
            }
                break;

            case 'W':
                if (!str_cmp(word, "Wearloc")) {
                    char *str = fread_string(fp);
                    pet->wearloc.fromString(str);
                    free_string(str);
                    fMatch = true;
                    break;
                }
                break;
            }

            if ( !fMatch )
            {
                bug("Fread_pet: no match.",0);
                fread_to_eol(fp);
            }
            
    }
}

/* load a mobile from the forgotten reaches */
NPCharacter * fread_mob( FILE *fp )
{
    const char *word;
    NPCharacter *mob;
    bool fMatch;
    bitstring_t create_flags = FCREATE_NOAFFECTS | FCREATE_NOCOUNT;

    // first entry had BETTER be the vnum or we barf
    word = feof(fp) ? "End" : fread_word(fp);

    if ( !str_cmp(word,"Vnum") )
    {
            int vnum;
    
            vnum = fread_number(fp);
            if ( get_mob_index (vnum ) == 0 )
            {
                    bug("Fread_mob: bad vnum %d.",vnum);
                    mob = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
            }
            else
                    mob = create_mobile_org(get_mob_index(vnum), create_flags);
    }
    else
    {
            bug("Fread_mob: no vnum in file.",0);
            mob = create_mobile_org(get_mob_index(MOB_VNUM_FIDO), create_flags);
    }

    mob->pIndexData->count++;
    mob->affected.deallocate();

    try {
        for ( ; ; )
        {
            word         = feof(fp) ? "End" : fread_word(fp);

            fMatch = false;

            switch (dl_toupper(word[0]))
            {
            case '*':
                    fMatch = true;
                    fread_to_eol(fp);
                    break;

            case 'A':
                    KEY( "Act",                mob->act,                fread_flag(fp));
                    KEY( "AfBy",        mob->affected_by,        fread_flag(fp));
                    KEY( "Alig",        mob->alignment,                fread_number(fp));

                    if ( !str_cmp(word,"ACs") )
                    {
                            int i;

                            for ( i = 0; i < 4; i++ )
                                    mob->armor[i] = fread_number(fp);
                            fMatch = true;
                            break;
                    }

                    if ( !str_cmp(word,"Affc") )
                    {
                            Affect *af = fread_affect( fp );
                            
                            affect_to_char( mob, af );
                            ddeallocate( af );
                            fMatch          = true;
                            break;
                    }

                    if (!str_cmp(word,"AMod"))
                    {
                            int stat;

                            for (stat = 0; stat < stat_table.size; stat++)
                                    mob->mod_stat[stat] = fread_number(fp);
                            fMatch = true;
                            break;
                    }

                    if (!str_cmp(word,"Attr"))
                    {
                            int stat;

                            for (stat = 0; stat < stat_table.size; stat++)
                                    mob->perm_stat[stat] = fread_number(fp);
                            fMatch = true;
                            break;
                    }
                    break;

            case 'C':
                    KEY( "Comm",        mob->comm,                fread_flag(fp));

                    if (!str_cmp( word, "Cab" )) {
                        fread_number( fp );
                        fMatch = true;
                        break;
                    }

                    if (!str_cmp(word, "Clan")) {
                        char *word = fread_string(fp);
                        Clan *clan = clanManager->findExisting(word);
                        if (clan)
                            mob->setClan(clan->getName());

                        fMatch = true;
                        free_string( word );
                        break;
                    }

                    break;

            case 'D':
                    KEY( "Dam",        mob->damroll,                fread_number(fp));
                    KEY( "DamT",       mob->damage[DICE_TYPE],      fread_number(fp));
                    KEY( "DamN",       mob->damage[DICE_NUMBER],    fread_number(fp));
                    KEY( "Detect",        mob->detection,                fread_flag(fp));

                    if (!str_cmp( word, "Desc" )) {
                        char *word = fread_string( fp );
                        mob->setDescription( word );
                        free_string( word );
                        fMatch = true;
                        break;
                    }
                    break;

            case 'E':
                    if (!str_cmp(word,"End"))
                    {
                            mob->leader = 0;
                            mob->master = 0;

                            MobileBehaviorManager::parse( mob, fp );
                            if (FeniaManager::wrapperManager)
                                FeniaManager::wrapperManager->linkWrapper( mob );
                            return mob;
                    }

                    KEY( "Exp",        mob->exp,                fread_number(fp));
                    break;

            case 'G':
                    KEY( "Gold",        mob->gold,                fread_number(fp));
                    break;

            case 'H':
                    KEY( "Hit",        mob->hitroll,                fread_number(fp));

                    if (!str_cmp(word,"HMV"))
                    {
                            mob->hit        = fread_number(fp);
                            mob->max_hit        = fread_number(fp);
                            mob->mana        = fread_number(fp);
                            mob->max_mana        = fread_number(fp);
                            mob->move        = fread_number(fp);
                            mob->max_move        = fread_number(fp);
                            fMatch = true;
                            break;
                    }
                    break;

            case 'I':
                    if (!str_cmp( word, "Id" )) {
                        mob->setID( fread_number64( fp ) );
                        fMatch = true;
                        break;
                    }

                    break;

            case 'L':
                    if ( !str_cmp( word, "Levl" ) )
                    {
                            mob->setLevel( fread_number( fp ) );
                            fMatch = true;
                            break;
                    }

                    if (!str_cmp( word, "LnD" )) {
                        char *word = fread_string( fp );
                        mob->setLongDescr( word );
                        fMatch = true;
                        free_string( word );
                        break;
                    }
                    break;

            case 'N':
                    if ( !str_cmp( word, "Name" ) )
                    {
                            char *nm = fread_string( fp );
                            DLString name( nm );
                            free_string(nm);
                            // For non-renamed mobs, update their name from index data,
                            // as it can be translated or altered at this point.
                            if (is_name(name.c_str(), mob->pIndexData->player_name))
                                name = mob->pIndexData->player_name;
                            mob->setName( name );
                            fMatch = true;
                            break;
                    }
                    break;

            case 'P':
                    KEYV( "Pos",        mob->position,                fread_number(fp));
                    break;

            case 'R':
                    if (!str_cmp( word, "Room" )) {
                        mob->in_room = get_room_instance( fread_number( fp ) );
                        fMatch = true;
                        break;
                    }

                    if ( !str_cmp( word, "Race" ) )
                    {
                            char *rnm = fread_string(fp);
                            mob->setRace( rnm );
                            free_string(rnm);

                            fMatch = true;
                            break;
                    }

                    if ( !str_cmp( word, "RZone" ) )
                    {
                            char *rznm = fread_string( fp );
                            DLString zoneName(rznm);
                            free_string(rznm);
                            AreaIndexData *pArea = get_area_index(zoneName);
                            if (pArea) {
                                mob->zone = pArea;
                                fMatch = true;
                                break;
                            }
                    }

                    KEY("RRoom", mob->reset_room, fread_number(fp));
                    break;

            case 'S' :
                    KEY( "Save",        mob->saving_throw,        fread_number(fp));
                    KEY( "Silv",        mob->silver,            fread_number( fp ) );

                    if( !str_cmp( word, "Sex" ) )
                    {
                            int sex = fread_number( fp );

                            if (sex_table.name( sex ).empty( ))
                                mob->setSex( mob->pIndexData->sex );
                            else
                                mob->setSex( sex );

                            if (mob->getSex( ) == SEX_EITHER)
                                mob->setSex( number_range( 1, 2 ) );

                            fMatch = true;
                            break;
                    }
                    if (!str_cmp( word, "ShD" )) {
                        char *word = fread_string( fp );
                        mob->setShortDescr( word );
                        fMatch = true;
                        free_string( word );
                        break;
                    }
                    break;

            case 'T':
                    KEY( "Timer",        mob->timer,                fread_number( fp ) );
                    break;

            case 'W':
                if (!str_cmp(word, "Wearloc")) {
                    char *str = fread_string(fp);
                    mob->wearloc.fromString(str);
                    free_string(str);
                    fMatch = true;
                    break;
                }
                break;

            }
            
            if ( !fMatch )
            {
                    bug("Fread_mob: no match.",0);
                    fread_to_eol(fp);
            }
        }

    } catch (const FileFormatException &e) {
        extract_mob_baddrop( mob );
        throw e;
    }
    
}

void fread_mlt( PCharacter *ch, FILE *fp ) {
  bool fMatch;
  const char *word;
  int i;

  word   = feof( fp ) ? "End" : fread_word( fp );
  if( str_cmp( word, "MLTLv" ) ) {
    bug( "fread_mlt: no remort count.", 0 );
    fread_to_eol( fp );
    return;
  }

  fread_number( fp );
  i = -1;

  for( ; ; ) {
    word   = feof( fp ) ? "End" : fread_word( fp );
    if( word[0] == '#' ) {
      i++;
      word   = feof( fp ) ? "End" : fread_word( fp );
    }
    if( i < 0 ) i++; // на всяк случай
    switch( dl_toupper( word[0] ) ) {
      case '*':
        fMatch = true;
        fread_to_eol( fp );
        break;
      case 'C':
        KEYSKIP( "Class" );
        break;
      case 'E':
        if( !str_cmp( word, "End" ) ) {
          return;
        }
        break;
      case 'R':
        KEYSKIP( "Race" );
        break;
      case 'T':
        KEYSKIP( "Time" );
        break;
      if( !fMatch ) {
          bug( "fread_mlt: no match.", 0 );
          fread_to_eol( fp );
      }
    }
  }
}

static void convert_personal( Object *obj )
{
    AllocateClass::Pointer pointer;
    ObjectBehavior::Pointer behavior;
 
    if (obj->pIndexData->vnum == OBJ_VNUM_KATANA_SWORD) 
        pointer = Class::allocateClass( "OwnedKatana" );
    else
        pointer = Class::allocateClass( "PersonalQuestReward" );
        
    behavior = pointer.getDynamicPointer<ObjectBehavior>( );
    obj->behavior.setPointer( *behavior );
    obj->behavior->setObj( obj );

    LogStream::sendNotice( ) 
            << "Personal: obj [" << obj->pIndexData->vnum << "], "
            << "ID " << obj->getID( ) << ", owner " << obj->getOwner( ) << endl;
}

#define OBJ_VNUM_STUB 127

void fread_obj( Character *ch, Room *room, FILE *fp )
{
    Object *obj;
    const char *word;
    int iNest;
    bool fMatch = true;
    bool fNest;
    bool fVnum;
    bool first;
    bool fPersonal;
    int wear_loc = -1;
    int vnum = 0;
    AffectList affectsOldStyle;

    fVnum = false;
    obj = 0;
    first = true;  /* used to counter fp offset */
    fPersonal = false;

    word   = feof( fp ) ? "End" : fread_word( fp );

    if( !str_cmp( word,"Vnum" ) )
    {
            first = false;  /* fp will be in right place */

            vnum = fread_number( fp );
            OBJ_INDEX_DATA *pObj = get_obj_index(vnum);

            if (!pObj)
            {
                bug( "Fread_obj: bad vnum %d in room %d.", vnum, room ? room->vnum : -1 );
                pObj = get_obj_index(OBJ_VNUM_STUB);
            }

            /*this object was already initialized once*/
            obj = create_object_nocount( pObj, -1);
            
            /*init pIndexData counter, in case of bootup*/
            if (create_obj_dropped)
                obj->pIndexData->count++;
    }

    if ( obj == 0 )
    {
        bug( "Fread_obj: zero object", 0 );
        return;
    }

    fNest                = false;
    fVnum                = true;
    iNest                = 0;

    try {

        for ( ; ; )
        {
            if ( first )
                    first = false;
            else
                    word   = feof( fp ) ? "End" : fread_word( fp );
            fMatch = false;

            switch ( dl_toupper(word[0]) )
            {
            case '*':
                    fMatch = true;
                    fread_to_eol( fp );
                    break;

            case 'A':
                    KEYSKIP( "Altar" );
                    if (!str_cmp(word,"Affc"))
                    {
                            Affect *paf = fread_affect( fp );
                            affectsOldStyle.push_front(paf);
                            fMatch          = true;
                            break;
                    }
                    if (!str_cmp(word, "Affect"))
                    {
                            Affect *paf = fread_affect( fp );
                            obj->affected.push_front(paf);
                            fMatch          = true;
                            break;
                    }

                    break;

            case 'C':
                    KEY( "Cond",        obj->condition,                fread_number( fp ) );
                    KEY( "Cost",        obj->cost,                fread_number( fp ) );
                    break;

            case 'D':
                    if (!str_cmp( word, "Description" ) || !str_cmp( word, "Desc" )) {
                        char *word = fread_string( fp );
                        obj->setDescription( word );
                        free_string( word );
                        fMatch = true;
                        break;
                    }
                    break;

            case 'E':
                    if ( !str_cmp( word, "Enchanted"))
                    {
                            // Obsolete logic for working with item instance affects.
                            fMatch         = true;
                            break;
                    }

                    KEY( "ExtraFlags",        obj->extra_flags,        fread_number( fp ) );
                    KEY( "ExtF",        obj->extra_flags,        fread_number( fp ) );

                    if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe"))
                    {
                            EXTRA_DESCR_DATA *ed;

                            ed = new_extra_descr();

                            ed->keyword                = fread_string( fp );
                            ed->description                = fread_string( fp );
                            ed->next                = obj->extra_descr;
                            obj->extra_descr        = ed;
                            fMatch = true;
                    }

                    if ( !str_cmp( word, "End" ) )
                    {

                        if ( !fNest || !fVnum || obj->pIndexData == 0)
                        {
                            bug( "Fread_obj: incomplete object.", 0 );
                            ddeallocate( obj );
                            return;
                        }
                        else if (obj->pIndexData->vnum == OBJ_VNUM_STUB) {
                            bug("Fread_obj: extracting stub item %d in room %d", vnum, room ? room->vnum : -1);
                            ObjectBehaviorManager::parse( obj, fp );                            
                            extract_obj( obj );
                            return;
                        }
                        else
                        {
                            // Object affect conversion: getting rid of 'enchanted' logic when proto affects got duplicated on the instance.
                            if (!affectsOldStyle.empty()) {
                                // Random weapons and katanas don't have any other affects on the proto 
                                // so it's safe to add all instance affects back as they were.
                                bool isKatana = any_of(affectsOldStyle.begin(), affectsOldStyle.end(), 
                                                [&](const Affect* paf){ return paf->type.getName() == gsn_katana->getName(); });
                                bool isRandom = !obj->getProperty("tier").empty();

                                if (isKatana || isRandom) {
                                    notice("Fread_obj: retained %d proper affects for %d [%lld]", affectsOldStyle.size(), obj->pIndexData->vnum, obj->getID());
                                    for (auto &paf: affectsOldStyle)
                                        obj->affected.push_back(paf);
                                } else {
                                    // For all other items, retain only temporary affects as those are harmless 
                                    // and may add important bits such as 'noenchant' that we want removed eventually.
                                    for (auto &paf: affectsOldStyle)
                                        if (paf->duration >= 0)
                                            obj->affected.push_back(paf);
                                }
                            }

                            Object *container = 0;
                            if (iNest > 0 && rgObjNest[iNest])
                                container = rgObjNest[iNest-1];

                            if (container)
                                obj_to_obj(obj, container);
                            else if (ch)
                                obj_to_char(obj, ch);
                            else
                                obj_to_room(obj, room);

                            ObjectBehaviorManager::parse( obj, fp );
                            
                            if (FeniaManager::wrapperManager)
                                FeniaManager::wrapperManager->linkWrapper( obj );

                            // Notify item load listeners such as weapon randomizer.
                            eventBus->publish(ItemReadEvent(obj));

                            if (fPersonal) 
                                convert_personal( obj );

                            if (wear_loc != -1)
                                obj->wear_loc.assign(wear_loc);

                            return;
                        }
                    }
                    break;

            case 'G':
                    if (!str_cmp( word, "Gender" )) {
                        obj->gram_gender.fromString(fread_word( fp ));
                        obj->updateCachedNoun();
                        fMatch = true;
                        break;
                    }

                    break;

            case 'I':
                    KEY( "ItemType",        obj->item_type,                fread_number( fp ) );
                    KEY( "Ityp",        obj->item_type,                fread_number( fp ) );
                    
                    if (!str_cmp( word, "Id" )) {
                        obj->setID( fread_number64( fp ) );
                        fMatch = true;
                        break;
                    }

                    break;

            case 'L':
                    KEY( "Level",        obj->level,                fread_number( fp ) );
                    KEY( "Lev",                obj->level,                fread_number( fp ) );
                    if (!str_cmp( word, "Liquid" )) {
                        obj->value2(liquidManager->lookup(fread_word(fp)));
                        fMatch = true;
                        break;
                    }
                    break;

            case 'M':
                    if (!str_cmp( word, "Material" )) {
                        char *word = fread_string( fp );
                        obj->setMaterial( word );
                        fMatch = true;
                        free_string( word );
                    }
                    break;

            case 'N':
                    if (!str_cmp( word, "Name" )) {
                        char *word = fread_string( fp );
                        obj->setName( word );
                        fMatch = true;
                        free_string( word );
                        break;
                    }

                    if ( !str_cmp( word, "Nest" ) )
                    {
                            iNest = fread_number( fp );
                            if ( iNest < 0 || iNest >= MAX_NEST )
                            {
                                    bug( "Fread_obj: bad nest %d.", iNest );
                            }
                            else
                            {
                                    rgObjNest[iNest] = obj;
                                    fNest = true;
                            }
                            fMatch = true;
                    }
                    break;

            case 'O':
                    if( !str_cmp( word, "ObjPrg" ) )
                    {
                        DLString otype, oname;

                        otype = fread_word( fp );
                        oname = fread_word( fp );

                        if (otype == "get_prog" && oname == "get_prog_quest_reward")
                            fPersonal = true;

                        fMatch = true;
                        break;
                    }

                    if (!str_cmp( word, "Ownr" )) {
                        char *word = fread_string( fp );
                        obj->setOwner( word );
                        fMatch = true;
                        free_string( word );
                    }

                    break;

            case 'P':
                    KEY( "Pocket", obj->pocket, fread_string( fp ) );
                    KEYSKIP( "Pit" );
                    break;
            
            case 'Q':
                    KEY( "Quality",        obj->condition,                fread_number( fp ) );
                    break;

            case 'R':
                    KEY( "RRoom",         obj->reset_room,        fread_number( fp ) );
                    KEY( "RMob",          obj->reset_mob,         fread_number64( fp ) );
                    KEY( "RObj",          obj->reset_obj,         fread_number64( fp ) );
                    break;

            case 'S':
                    if (!str_cmp( word, "ShortDescr" ) || !str_cmp( word, "ShD" )) {
                        char *word = fread_string( fp );
                        obj->setShortDescr( word );
                        fMatch = true;
                        free_string( word );
                        break;
                    }

                    if ( !str_cmp( word, "Spell" ) )
                    {
                            int iValue;
                            int sn;

                            iValue = fread_number( fp );
                            sn     = SkillManager::getThis( )->lookup( fread_word( fp ) );
                            convert_skill( sn );

                            if ( iValue < 0 || iValue > 4 )
                            {
                                    bug( "Fread_obj: bad iValue %d.", iValue );
                            }
                            else if ( sn < 0 )
                            {
                                    bug( "Fread_obj: unknown skill.", 0 );
                            }
                            else
                            {
                                    obj->valueByIndex(iValue, sn);
                            }
                            fMatch = true;
                            break;
                    }
 
                    break;

            case 'T':
                    KEY( "TS",          obj->timestamp,         fread_number64( fp ) );
                    KEY( "Timer",        obj->timer,                fread_number( fp ) );
                    KEY( "Time",        obj->timer,                fread_number( fp ) );
                    break;

            case 'V':
                    if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals"))
                    {
                            obj->value0(fread_number( fp ));
                            obj->value1(fread_number( fp ));
                            obj->value2(fread_number( fp ));
                            obj->value3(fread_number( fp ));
                            if (obj->item_type == ITEM_WEAPON && obj->value0() == 0)
                                    obj->value0(obj->pIndexData->value[0]);
                            convert_obj_values( obj );
                            fMatch                = true;
                            break;
                    }

                    if ( !str_cmp( word, "Val" ) )
                    {
                            obj->value0(fread_number( fp ));
                            obj->value1(fread_number( fp ));
                            obj->value2(fread_number( fp ));
                            obj->value3(fread_number( fp ));
                            obj->value4(fread_number( fp ));
                            convert_obj_values( obj );
                            fMatch = true;
                            break;
                    }

                    if ( !str_cmp( word, "Vnum" ) )
                    {
                            int vnum;

                            vnum = fread_number( fp );
                            if ( ( obj->pIndexData = get_obj_index( vnum ) ) == 0 )
                                    bug( "Fread_obj: bad vnum %d.", vnum );
                            else
                                    fVnum = true;
                            fMatch = true;
                            break;
                    }
                    break;

            case 'W':
                    KEY( "WearFlags",        obj->wear_flags,        fread_number( fp ) );
                    KEY( "WeaF",        obj->wear_flags,        fread_number( fp ) );
                    KEY( "Wear",        wear_loc,                fread_number( fp ) );
                    KEY( "Weight",        obj->weight,                fread_number( fp ) );
                    KEY( "Wt",                obj->weight,                fread_number( fp ) );

                    if (!str_cmp( word, "Wearloc" )) {
                        Wearlocation *wearloc = wearlocationManager->find( fread_word( fp ) );
                        wear_loc = wearloc ? wearloc->getIndex() : -1;
                        fMatch = true;
                        break;
                    }

                    break;
            case 'X':
                    if (!str_cmp(word, "X")) {
                        DLString key = fread_word(fp);
                        obj->properties[key] = fread_dlstring(fp);
                        fMatch = true;
                        break;
                    }
                    break;
            }

            if ( !fMatch )
            {
                    bug( "Fread_obj: no match.", 0 );
                    LogStream::sendNotice() << "word:" << word << ":" << endl;
                    fread_to_eol( fp );
            }
        }

    } catch (const FileFormatException &e) {
        extract_obj( obj );
        throw e;
    }
}


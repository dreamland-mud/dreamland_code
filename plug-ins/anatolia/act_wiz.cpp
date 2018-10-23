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
#include "so.h"
#include "plugin.h"
#include "char.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skill.h"
#include "skillmanager.h"
#include "affecthandler.h"

#include "plugininitializer.h"
#include "logstream.h"
#include "dlfileop.h"
#include "dreamland.h"
#include "affect.h"
#include "room.h"
#include "pcharactermanager.h"
#include "race.h"
#include "pcharactermemory.h"
#include "mobilebehavior.h"
#include "objectbehavior.h"
#include "pcharacter.h"
#include "save.h"
#include "merc.h"
#include "descriptor.h"
#include "desire.h"
#include "helpmanager.h"
#include "comm.h"
#include "badnames.h"
#include "wiznet.h"
#include "mercdb.h"
#include "interp.h"
#include "clan.h"
#include "liquid.h"

#include "worldknowledge.h"
#include "gsn_plugin.h"
#include "npcharacter.h"
#include "object.h"
#include "handler.h"
#include "fread_utils.h"
#include "stats_apply.h"
#include "act_wiz.h"
#include "act_move.h"
#include "act.h"
#include "def.h"

using std::min;

/* dedicated loader for wizard commands */
CMDLOADER_DECL(wizard)


/* command procedures needed */
void do_rstat                ( Character *, char * );
void do_mstat                ( Character *, Character * );
void do_ostat                ( Character *, char * );
void do_mfind                ( Character *, char * );
void do_ofind                ( Character *, char * );
void do_tfind                ( Character *, char * );
void do_mload                ( Character *, char * );
void do_oload                ( Character *, char * );

RELIG(none);

/*
 * Local functions.
 */
static Room * find_location( Character *ch, char *arg )
{
    Character *victim;
    Object *obj;

    if ( is_number(arg) )
        return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != 0 )
        return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != 0 )
        return obj->in_room;

    return 0;
}



CMDWIZP( objlist )
{
FILE *fp;
Object *obj;
Affect *paf;
Liquid *liquid;
int currLevel, SetL = 0;
char arg[MAX_STRING_LENGTH];

   if ( (fp=fopen( "objlist.txt", "w+" ) ) == 0 )
   {
        ch->send_to("File error.\n\r");
        return;
   }

   argument = one_argument( argument, arg );
   if ((SetL = atoi(arg))==0)
     SetL = 110;

  for(currLevel=SetL; currLevel>=0; currLevel-- ) {
   fprintf( fp, "\n======= LEVEL %d ========\n", currLevel );
   for( obj=object_list; obj!=0; obj = obj->next )
   {
/*     if ( obj->pIndexData->affected != 0 )                */
       if ( obj->level==currLevel )        
     {
       fprintf( fp, "\n#Obj: %s (Vnum : %d) \n", obj->getShortDescr( ) ,obj->pIndexData->vnum);
    fprintf( fp,
        "Object '%s' is type %s, extra flags %s.\nWeight is %d, value is %d, level is %d.\n",

        obj->getName( ),
        item_table.message(obj->item_type).c_str( ),
        extra_flags.messages( obj->extra_flags ).c_str( ),
        obj->weight / 10,
        obj->cost,
        obj->level
        );

    switch ( obj->item_type )
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        fprintf( fp, "Level %d spells of:", obj->value[0] );

        if ( obj->value[1] >= 0 && obj->value[1] < SkillManager::getThis( )->size() )
        {
            fprintf(fp, " '%s'", SkillManager::getThis( )->find(obj->value[1])->getName().c_str());
        }

        if ( obj->value[2] >= 0 && obj->value[2] < SkillManager::getThis( )->size() )
        {
            fprintf(fp, " '%s'", SkillManager::getThis( )->find(obj->value[2])->getName().c_str());
        }

        if ( obj->value[3] >= 0 && obj->value[3] < SkillManager::getThis( )->size() )
        {
            fprintf(fp, " '%s'", SkillManager::getThis( )->find(obj->value[3])->getName().c_str());
        }

        if (obj->value[4] >= 0 && obj->value[4] < SkillManager::getThis( )->size())
        {
            fprintf(fp, " '%s'", SkillManager::getThis( )->find(obj->value[4])->getName().c_str());
        }

        fprintf( fp,".\n");
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        fprintf(fp, "Has %d charges of level %d", obj->value[2], obj->value[0]);

        if ( obj->value[3] >= 0 && obj->value[3] < SkillManager::getThis( )->size() )
        {
            fprintf(fp, " '%s'", SkillManager::getThis( )->find(obj->value[3])->getName().c_str());
        }

        fprintf( fp,".\n");
        break;

    case ITEM_DRINK_CON:
        liquid = liquidManager->find( obj->value[2] );
        fprintf(fp,"It holds %s-colored %s.\n",
                    liquid->getColor( ).ruscase( '2' ).c_str( ),
                    liquid->getShortDescr( ).ruscase( '4' ).c_str( ) );
        break;

    case ITEM_CONTAINER:
        fprintf(fp,"Capacity: %d#  Maximum weight: %d#  flags: %s\n",
            obj->value[0], obj->value[3], container_flags.messages(obj->value[1]).c_str( ));
        if (obj->value[4] != 100)
        {
            fprintf(fp,"Weight multiplier: %d%%\n",
                obj->value[4]);
        }
        break;
                
    case ITEM_WEAPON:
         fprintf(fp,"Weapon type is %s\n", 
                    weapon_class.name(obj->value[0]).c_str( ));
                
        if (obj->pIndexData->new_format)
            fprintf(fp,"Damage is %dd%d (average %d).\n",
                obj->value[1],obj->value[2],
                (1 + obj->value[2]) * obj->value[1] / 2);
        else
            fprintf( fp, "Damage is %d to %d (average %d).\n",
                    obj->value[1], obj->value[2],
                    ( obj->value[1] + obj->value[2] ) / 2 );
        if (obj->value[4])  /* weapon flags */
        {
            fprintf(fp,"Weapons flags: %s\n",weapon_type2.messages(obj->value[4]).c_str( ));
        }
        break;

    case ITEM_ARMOR:
        fprintf( fp,
        "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n",
            obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
        break;
    }
       for( paf=obj->pIndexData->affected; paf != 0; paf = paf->next )
       {
            if ( paf == 0 ) continue;
            fprintf( fp, "  Affects %s by %d.\n",
                apply_flags.message( paf->location ).c_str( ), paf->modifier );
            if (paf->bitvector)
            {
                switch(paf->where)
                {
                    case TO_AFFECTS:
                        fprintf(fp,"   Adds %s affect.\n",
                            affect_flags.messages(paf->bitvector).c_str( ));
                        break;
                    case TO_OBJECT:
                        fprintf(fp,"   Adds %s object flag.\n",
                            extra_flags.messages(paf->bitvector).c_str( ));
                        break;
                    case TO_IMMUNE:
                        fprintf(fp,"   Adds immunity to %s.\n",
                            imm_flags.messages(paf->bitvector).c_str( ));
                        break;
                    case TO_RESIST:
                        fprintf(fp,"   Adds resistance to %s.\n\r",
                            res_flags.messages(paf->bitvector).c_str( ));
                        break;
                    case TO_VULN:
                        fprintf(fp,"   Adds vulnerability to %s.\n\r",
                            vuln_flags.messages(paf->bitvector).c_str( ));
                        break;
                    case TO_DETECTS:
                        fprintf(fp,"   Adds %s detection.\n\r",
                            detect_flags.messages(paf->bitvector).c_str( ));
                        break;
                    default:
                        fprintf(fp,"   Unknown bit %d: %d\n\r",
                            paf->where,paf->bitvector);
                        break;
                }
            }
       }
     }
   }
  }
   fclose( fp );
   return;
}

CMDWIZP( limited )
{
        extern int top_obj_index;
        Object *obj;
        OBJ_INDEX_DATA *obj_index;
        int        lCount = 0;
        int        ingameCount;
        char  buf[1000];
        int         nMatch;
        int         vnum;
        ostringstream report;

        if ( argument[0] != '\0' )
        {
                obj_index = get_obj_index( atoi(argument) );
                if ( obj_index == 0 )
                {
                        ch->send_to("Not found.\n\r");
                        return;
                }
                if ( obj_index->limit == -1 )
                {
                        ch->send_to("Thats not a limited item.\n\r");
                        return;
                }
                nMatch = 0;
                sprintf( buf, "%-35s [%5d]  Limit: %3d  Current: %3d\n\r",
                        obj_index->short_descr,
                        obj_index->vnum,
                        obj_index->limit,
                        obj_index->count );
                buf[0] = Char::upper( buf[0] );
                ch->send_to(buf);
                ingameCount = 0;
                for ( obj=object_list; obj != 0; obj=obj->next )
                        if ( obj->pIndexData->vnum == obj_index->vnum )
                        {
                                ingameCount++;
                                if ( obj->carried_by != 0 && ch->can_see( obj->carried_by ) )
                                        sprintf(buf, "Carried by %-30s\n\r",
                                                obj->carried_by->getNameP( ));
                                if ( obj->in_room != 0 )
                                        sprintf(buf, "At %-20s [%d]\n\r",
                                                obj->in_room->name, obj->in_room->vnum);
                                if ( obj->in_obj != 0 )
                                        sprintf(buf, "In %-20s [%d] \n\r",
                                                obj->in_obj->getShortDescr( '1' ).c_str( ),
                                                obj->in_obj->pIndexData->vnum);
                                        ch->send_to(buf);
                        }
                sprintf(buf, "  %d found in game. %d should be in pFiles.\n\r",
                        ingameCount, obj_index->count-ingameCount);
                ch->send_to(buf);
                return;
        }

        nMatch = 0;
        for ( vnum = 0; nMatch < top_obj_index; vnum++ )
            if ( ( obj_index = get_obj_index( vnum ) ) != 0 )
            {
                nMatch++;
                if (obj_index->limit > 0 && obj_index->limit < 100 && obj_index->count > 0)
                {
                    int inGame = 0;
                    for (Object* obj=object_list; obj != 0; obj=obj->next )
                            if ( obj->pIndexData->vnum == obj_index->vnum )
                                inGame++;

                    lCount++;
                    ch->pecho( "%-36^N1 [%5d]  Limit: %3d  Current: %3d In Game: %3d",
                            obj_index->short_descr,
                            obj_index->vnum,
                            obj_index->limit,
                            obj_index->count,
                            inGame );

                    report << obj_index->vnum << " ";
                }
            }
        sprintf( buf, "\n\r%d of %d objects are limited.\n\r", lCount, nMatch );
        ch->send_to(buf);
        ch->println(report.str( ));
        return;
}

CMDWIZP( wiznet )
{
        int flag;
        char buf[MAX_STRING_LENGTH];

        if (!ch->getPC( ))
            return;

        if ( argument[0] == '\0' )
        {
                if (IS_SET(ch->getPC( )->wiznet,WIZ_ON))
                {
                        ch->send_to("Signing off of Wiznet.\n\r");
                        REMOVE_BIT(ch->getPC( )->wiznet,WIZ_ON);
                }
                else
                {
                        ch->send_to("Welcome to Wiznet!\n\r");
                        SET_BIT(ch->getPC( )->wiznet,WIZ_ON);
                }

                return;
        }

        if (!str_prefix(argument,"on"))
        {
                ch->send_to("Welcome to Wiznet!\n\r");
                SET_BIT(ch->getPC( )->wiznet,WIZ_ON);
                return;
        }

        if (!str_prefix(argument,"off"))
        {
                ch->send_to("Signing off of Wiznet.\n\r");
                REMOVE_BIT(ch->getPC( )->wiznet,WIZ_ON);
                return;
        }

        /* show wiznet status */
        if (!str_prefix(argument,"status"))
        {
                buf[0] = '\0';

                if (!IS_SET(ch->getPC( )->wiznet,WIZ_ON))
                        strcat(buf,"off ");

                for (flag = 0; wiznet_table[flag].name != 0; flag++)
                        if (IS_SET(ch->getPC( )->wiznet,wiznet_table[flag].flag))
                        {
                                strcat(buf,wiznet_table[flag].name);
                                strcat(buf," ");
                        }

                strcat(buf,"\n\r");

                ch->send_to("Wiznet status:\n\r");
                ch->send_to(buf);
                return;
        }

        if ( !str_prefix(argument,"show") )
        {
                /* list of all wiznet options */
                buf[0] = '\0';

                for (flag = 0; wiznet_table[flag].name != 0; flag++)
                {
                        if (wiznet_table[flag].level <= ch->get_trust())
                        {
                                strcat(buf,wiznet_table[flag].name);
                                strcat(buf," ");
                        }
                }

                strcat(buf,"\n\r");

                ch->send_to("Wiznet options available to you are:\n\r");
                ch->send_to(buf);

                return;
        }

        flag = wiznet_lookup(argument);

        if ( flag == -1 || ch->get_trust() < wiznet_table[flag].level )
        {
                ch->send_to("No such option.\n\r");
                return;
        }

        if ( IS_SET(ch->getPC( )->wiznet,wiznet_table[flag].flag) )
        {
                ch->printf("You will no longer see %s on wiznet.\n\r",
                           wiznet_table[flag].name);
                REMOVE_BIT(ch->getPC( )->wiznet,wiznet_table[flag].flag);
                return;
        }
        else
        {
                ch->printf("You will now see %s on wiznet.\n\r",
                           wiznet_table[flag].name);
                SET_BIT(ch->getPC( )->wiznet,wiznet_table[flag].flag);
                return;
        }
}



CMDWIZP( poofin )
{
    if (ch->is_npc())
        return;

    if (argument[0] == '\0')
    {
            ch->printf("Your poofin is %s\n\r",ch->getPC( )->bamfin.c_str( ));
            return;
    }

    if ( strstr(argument,ch->getNameP( )) == 0 
            && strstr(argument,ch->getNameP( '1' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '2' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '3' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '4' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '5' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '6' ).c_str()) == 0 )
    {
            ch->send_to("You must include your name.\n\r");
            return;
    }

    ch->getPC( )->bamfin = argument;
    ch->printf("Your poofin is now %s\n\r",ch->getPC( )->bamfin.c_str( ));
}



CMDWIZP( poofout )
{
    if (ch->is_npc())
        return;

    if (argument[0] == '\0')
    {
            ch->printf("Your poofout is %s\n\r",ch->getPC( )->bamfout.c_str( ));
            return;
    }

    if ( strstr(argument,ch->getNameP( )) == 0 
            && strstr(argument,ch->getNameP( '1' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '2' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '3' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '4' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '5' ).c_str()) == 0 
            && strstr(argument,ch->getNameP( '6' ).c_str()) == 0 )
    {
            ch->send_to("You must include your name.\n\r");
            return;
    }

    ch->getPC( )->bamfout = argument;

    ch->printf("Your poofout is now %s\n\r",ch->getPC( )->bamfout.c_str( ));
}

CMDWIZP( disconnect )
{
    char arg[MAX_INPUT_LENGTH];
    Descriptor *d;
    Character *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->send_to("Disconnect whom?\n\r");
        return;
    }

    if (is_number(arg))
    {
        int desc;

        desc = atoi(arg);
            for ( d = descriptor_list; d != 0; d = d->next )
            {
            if ( d->descriptor == desc )
            {
                    d->close( );
                    ch->send_to("Ok.\n\r");
                    return;
            }
        }
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->desc == 0 )
    {
        act_p( "$C1 doesn't have a descriptor.", ch, 0, victim, TO_CHAR,POS_DEAD );
        return;
    }

    for ( d = descriptor_list; d != 0; d = d->next )
    {
        if ( d == victim->desc )
        {
            d->close( );
            ch->send_to("Ok.\n\r");
            return;
        }
    }

    bug( "Do_disconnect: desc not found.", 0 );
    ch->send_to("Descriptor not found!\n\r");
    return;
}


CMDWIZP( transfer )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Room *location;
    Descriptor *d;
    Character *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        ch->send_to("Transfer whom (and where)?\n\r");
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = descriptor_list; d != 0; d = d->next )
        {
            if ( d->connected == CON_PLAYING
            &&   d->character
            &&   d->character != ch
            &&   ch->can_see( d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->getNameP( ), arg2 );
                run( ch, buf );
            }
        }
        return;
    }


    if ( ( victim = get_char_world( ch, arg1 ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }
    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
        location = ch->in_room;
    }
    else
    {
        if ( ( location = find_location( ch, arg2 ) ) == 0 )
        {
            ch->send_to("No such location.\n\r");
            return;
        }
        

/*        if ( !location->isOwner(ch) && location ->isPrivate( ) */
        if ( location ->isPrivate( )
        &&  ch->get_trust() < MAX_LEVEL)
        {
            ch->send_to("That room is private right now.\n\r");
            return;
        }
    }
    
    if (victim->desc && victim->desc->connected != CON_PLAYING) {
        ch->println("It's a bad idea.");
        return;
    }

    transfer_char( victim, ch, location,
                  "%1$^C1 disappears in a mushroom cloud.",
                  (ch != victim ? "%2$^C1 has transferred you." : NULL),
                  "%1$^C1 arrives from a puff of smoke." );

    ch->send_to("Ok.\n\r");
}



CMDWIZP( at )
{
    char arg[MAX_INPUT_LENGTH];
    Room *location;
    Room *original;
    Object *on;
    Character *wch;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->send_to("At where what?\n\r");
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == 0 )
    {
        ch->send_to("No such location.\n\r");
        return;
    }

    if ( location ->isPrivate( ) &&  ch->get_trust() < MAX_LEVEL)
    {
        ch->send_to("That room is private right now.\n\r");
        return;
    }

    original = ch->in_room;
    on = ch->on;
    ch->dismount( );
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != 0; wch = wch->next )
    {
        if ( wch == ch )
        {
            undig( ch ); // handle 'at XXXX dig' case
            ch->dismount( );
            char_from_room( ch );
            char_to_room( ch, original );
            ch->on = on;
            break;
        }
    }

    return;
}


CMDWIZP( goto )
{
    Room *location;
    Character *rch;
    PCharacter *pch;

    if ( argument[0] == '\0' )
    {
        ch->send_to("Goto where?\n\r");
        return;
    }

    if ( ( location = find_location( ch, argument ) ) == 0 )
    {
        ch->printf("No such location: %s.\n\r", argument );
        return;
    }

    pch = ch->getPC( );

    if (!ch->is_npc( )) { // switched imms are silent
        for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
            if (rch->get_trust() >= ch->invis_level) {
                if (!pch->bamfout.empty( ))
                    act( "$t", ch, pch->bamfout.c_str( ), rch, TO_VICT );
                else
                    act( "$c1 leaves in a swirling mist.", ch, 0, rch, TO_VICT );
            }
    }
    
    transfer_char( ch, ch, location );
    
    if (!ch->is_npc( )) {
        for (rch = ch->in_room->people; rch != 0; rch = rch->next_in_room)
            if (rch->get_trust() >= ch->invis_level) {
                if (!pch->bamfin.empty( ))
                    act( "$t", ch, pch->bamfin.c_str( ), rch, TO_VICT );
                else
                    act( "$c1 appears in a swirling mist.", ch, 0, rch, TO_VICT );
            }
    }
}

/* RT to replace the 3 stat commands */

CMDWIZP( stat )
{
   char arg[MAX_INPUT_LENGTH];
   char *string;
   Object *obj;
   Room *location;
   Character *victim;
   bool fChar, fMob;

   string = one_argument(argument, arg);
   if ( arg[0] == '\0')
   {
        ch->send_to("Syntax:\n\r");
        ch->send_to("  stat <name>\n\r");
        ch->send_to("  stat obj <name>\n\r");
        ch->send_to("  stat mob <name>\n\r");
         ch->send_to("  stat room <number>\n\r");
        return;
   }

   if (!str_cmp(arg,"room"))
   {
        do_rstat(ch,string);
        return;
   }

   if (!str_cmp(arg,"obj"))
   {
        do_ostat(ch,string);
        return;
   }
    
    fChar = !str_cmp(arg,"char");
    fMob = !str_cmp(arg,"mob");

    if (fChar || fMob) {
        if (!string[0]) {
           ch->println("Stat на кого?");
           return;
        }
        
        victim =  fChar ? get_player_world( ch->getPC( ), string ) : get_char_world( ch, string );
        if (!victim) {
            ch->pecho("%s с таким именем не найден.", fMob ? "Персонаж" : "Игрок");
            return;
        }
        
        do_mstat(ch, victim);
        return;
   }

   /* do it the old way */

   obj = get_obj_world(ch,argument);
   if (obj != 0)
   {
     do_ostat(ch,argument);
     return;
   }

  victim = get_char_world(ch,argument);
  if (victim != 0)
  {
    do_mstat(ch, victim);
    return;
  }

  location = find_location(ch,argument);
  if (location != 0)
  {
    do_rstat(ch,argument);
    return;
  }

  ch->send_to("Nothing by that name found anywhere.\n\r");
}


/* NOTCOMMAND */ void do_rstat( Character *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    Room *location;
    Object *obj;
    Character *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == 0 )
    {
        ch->send_to("No such location.\n\r");
        return;
    }

/*    if (!location->isOwner(ch) && ch->in_room != location  */
    if ( ch->in_room != location
    &&  location ->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->send_to("That room is private right now.\n\r");
        return;
    }

    if (ch->in_room->affected_by)
    {
        sprintf(buf, "Affected by %s\n\r",
            raffect_flags.messages(ch->in_room->affected_by).c_str( ));
        ch->send_to(buf);
    }

    sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\rOwner: '%s' Clan: '%s'\n\r",
        location->name,
        location->area->name ,
        location->owner,
        location->clan->getShortName( ).c_str( ) );
    ch->send_to(buf);

    sprintf( buf,
        "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
        location->vnum,
        location->sector_type,
        location->light,
        location->heal_rate,
        location->mana_rate );
    ch->send_to(buf);

    sprintf( buf,
        "Room flags: %s (%ld).\n\rDescription:\n\r%s",
        room_flags.names(location->room_flags).c_str( ),
        location->room_flags,
        location->description );
    ch->send_to(buf);

    if ( location->extra_descr != 0 )
    {
        EXTRA_DESCR_DATA *ed;

        ch->send_to("Extra description keywords: '");
        for ( ed = location->extra_descr; ed; ed = ed->next )
        {
            ch->send_to(ed->keyword);
            if ( ed->next != 0 )
                ch->send_to(" ");
        }
        ch->send_to("'.\n\r");
    }

    ch->send_to("Characters:");
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
        if (ch->can_see(rch))
        {
            ch->printf( " %s", rch->getNameP( '1' ).c_str() );
        }
    }

    ch->send_to(".\n\rObjects:   ");
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
        ch->printf( " %s", obj->getFirstName( ).c_str( ) );
    }
    ch->send_to(".\n\r");

    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = location->exit[door] ) != 0 )
        {
            sprintf( buf,
                "Door: %d.  To: %d.  Key: %d. Level: %d.  Exit flags: %s.\n\rKeyword: '%s'.  Short: '%s'. Description: %s",

                door,
                ( pexit->u1.to_room == 0 ? -1 : pexit->u1.to_room->vnum),
                    pexit->key,
                pexit->level,
                    exit_flags.names(pexit->exit_info).c_str(),
                    pexit->keyword,
                direction_doorname(pexit),
                    pexit->description[0] != '\0'
                    ? pexit->description : "(none).\n\r" );
            ch->send_to(buf);
        }
    }
    ch->send_to("Tracks:\n\r");

    for (RoomHistory::iterator h = location->history.begin( );
         h != location->history.end( );
         h++)
    {
        ch->printf( "%s took door %d.\r\n", h->name.c_str( ), h->went );
    }

    if (location->behavior) {
        ostringstream ostr;
        
        sprintf(buf, "Behavior: [%s]\r\n", location->behavior->getType( ).c_str( ));
        ch->send_to(buf);

        location->behavior.toStream( ostr );
        ch->send_to( ostr );
    }

}



/* NOTCOMMAND */ void do_ostat( Character *ch, char *argument )
{
        char buf[MAX_STRING_LENGTH];
        char arg[MAX_INPUT_LENGTH];
        Affect *paf;
        Object *obj;
        Liquid *liquid;

        one_argument( argument, arg );

        if ( arg[0] == '\0' )
        {
                ch->send_to("Stat what?\n\r");
                return;
        }

        if ( ( obj = get_obj_world( ch, argument ) ) == 0 )
        {
                ch->send_to("Nothing like that in hell, earth, or heaven.\n\r");
                return;
        }

        sprintf( buf, "Name(s): %s\n\r", obj->getName( ) );
        ch->send_to(buf);

        sprintf( buf, "Vnum: %d  Limit: %d  Type: %s  Resets: %d\n\r",
                obj->pIndexData->vnum, obj->pIndexData->limit,
                item_table.message(obj->item_type).c_str( ),
                obj->pIndexData->reset_num );
        ch->send_to(buf);

        if (obj->timestamp > 0) {
            DLString d = Date( obj->timestamp ).getTimeAsString( );
            ch->printf("Лимит исчезнет в %s.\r\n", d.c_str( ) );
        }

        sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
                obj->getShortDescr( ), obj->getDescription( ) );
        ch->send_to(buf);

        sprintf(buf,"Owner: %s\n\r", obj->getOwner( ) == 0 ? "nobody" : obj->getOwner( ));
        ch->send_to(buf);

        sprintf( buf, "Material: %s\n\r", obj->getMaterial( ));
        ch->send_to(buf);

        sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
                wear_flags.messages(obj->wear_flags, true).c_str( ), 
                extra_flags.messages(obj->extra_flags, true).c_str( ) );
        ch->send_to(buf);

        sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",1,
                obj->getNumber( ), obj->weight, obj->getWeight( ), obj->getTrueWeight( ) );
        ch->send_to(buf);

        sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d Count: %d\n\r",
                obj->level, obj->cost, obj->condition, obj->timer, obj->pIndexData->count );
        ch->send_to(buf);

        sprintf( buf,        "In room: %d  In object: %s  Carried by: %s  Wear_loc: %s\n\r",
                obj->in_room == 0 ? 0 : obj->in_room->vnum,
                obj->in_obj  == 0 ? "(none)" : obj->in_obj->getShortDescr( '1' ).c_str( ),
                obj->carried_by == 0 ? "(none)" :
                        ch->can_see(obj->carried_by) ? obj->carried_by->getNameP( ) : "someone",
                obj->wear_loc->getName( ).c_str( ) );
        ch->send_to(buf);

        sprintf( buf, "Values: %d %d %d %d %d\n\r",
                obj->value[0], obj->value[1], obj->value[2], obj->value[3],        obj->value[4] );
        ch->send_to(buf);

        // now give out vital statistics as per identify

        switch ( obj->item_type )
        {
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_PILL:
                sprintf( buf, "Level %d spells of:", obj->value[0] );
                ch->send_to(buf);

                if ( obj->value[1] >= 0 && obj->value[1] < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value[1])->getName().c_str());
                        ch->send_to("'");
                }

                if ( obj->value[2] >= 0 && obj->value[2] < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value[2])->getName().c_str());
                        ch->send_to("'");
                }

                if ( obj->value[3] >= 0 && obj->value[3] < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value[3])->getName().c_str());
                        ch->send_to("'");
                }

                if (obj->value[4] >= 0 && obj->value[4] < SkillManager::getThis( )->size())
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value[4])->getName().c_str());
                        ch->send_to("'");
                }

                ch->send_to(".\n\r");
                break;

        case ITEM_WAND:
        case ITEM_STAFF:
                sprintf( buf, "Has %d(%d) charges of level %d",
                        obj->value[1], obj->value[2], obj->value[0] );
                ch->send_to(buf);

                if ( obj->value[3] >= 0 && obj->value[3] < SkillManager::getThis( )->size() )
                {
                        ch->send_to(" '");
                        ch->send_to(SkillManager::getThis( )->find(obj->value[3])->getName().c_str());
                        ch->send_to("'");
                }

                ch->send_to(".\n\r");
                break;

        case ITEM_DRINK_CON:
                liquid = liquidManager->find( obj->value[2] );
                sprintf(buf,"It holds %s-colored %s.\n",
                    liquid->getColor( ).ruscase( '2' ).c_str( ),
                    liquid->getShortDescr( ).ruscase( '4' ).c_str( ) );
                ch->send_to(buf);
                break;
                
        case ITEM_WEAPON:
                ch->send_to("Weapon type is ");
                ch->send_to(weapon_class.name(obj->value[0]).c_str( ));
                ch->send_to("\n");
                
                if (obj->pIndexData->new_format)
                        sprintf(buf,"Damage is %dd%d (average %d)\n\r",
                                obj->value[1],obj->value[2],(1 + obj->value[2]) * obj->value[1] / 2);
                else
                        sprintf( buf, "Damage is %d to %d (average %d)\n\r",
                                obj->value[1], obj->value[2],( obj->value[1] + obj->value[2] ) / 2 );
                        ch->send_to(buf);

                sprintf(buf,"Damage noun is %s.\n\r", weapon_flags.name(obj->value[3]).c_str( ));
                ch->send_to(buf);
        
                if (obj->value[4])  /* weapon flags */
                {
                        sprintf(buf,"Weapons flags: %s\n\r",weapon_type2.messages(obj->value[4]).c_str( ));
                        ch->send_to(buf);
                }
                break;

        case ITEM_ARMOR:
                sprintf( buf,"Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
                        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
                ch->send_to(buf);
                break;

        case ITEM_CONTAINER:
                sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
                        obj->value[0], obj->value[3], container_flags.messages(obj->value[1]).c_str( ));
                ch->send_to(buf);
                if (obj->value[4] != 100)
                {
                        sprintf(buf,"Weight multiplier: %d%%\n\r",obj->value[4]);
                        ch->send_to(buf);
                }
                break;
        
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
                ch->printf( "Steaks: %d, Level: %d, Parts: '%s', Vnum: %d\n\r",
                            obj->value[0], obj->value[1], 
                            part_flags.messages( obj->value[2] ).c_str( ), obj->value[3] );
                break;
        }

        if ( obj->extra_descr != 0 )
        {
                EXTRA_DESCR_DATA *ed;

                ch->send_to("Extra description keywords: '");

                for ( ed = obj->extra_descr; ed != 0; ed = ed->next )
                {
                        ch->send_to(ed->keyword);
                        if ( ed->next != 0 )
                                ch->send_to(" ");
                }

                ch->send_to ("'\n\r");
        }

        if ( obj->pIndexData->extra_descr != 0 )
        {
                EXTRA_DESCR_DATA *ed;

                ch->send_to("Extra description original: '");

                for ( ed = obj->pIndexData->extra_descr; ed != 0; ed = ed->next )
                {
                        ch->send_to(ed->keyword);
                        if ( ed->next != 0 )
                                ch->send_to(" ");
                }

                ch->send_to("'\n\r");
        }

    for ( paf = obj->affected; paf != 0; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d",
            apply_flags.message( paf->location ).c_str( ), paf->modifier,paf->level );
        ch->send_to(buf);
        if ( paf->duration > -1)
            sprintf(buf,", %d hours.\n\r",paf->duration);
        else
            sprintf(buf,".\n\r");
        ch->send_to(buf);
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_WEAPON:
                    sprintf(buf,"Adds %s weapon flags.\n",
                        weapon_type2.messages(paf->bitvector).c_str( ));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        res_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        vuln_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_DETECTS:
                    sprintf(buf,"Adds %s detection.\n\r",
                        detect_flags.messages(paf->bitvector).c_str( ));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            ch->send_to(buf);
        }
    }

    if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != 0; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d, level %d.\n\r",
            apply_flags.message( paf->location ).c_str( ), paf->modifier,paf->level );
        ch->send_to(buf);
        if (paf->bitvector)
        {
            switch(paf->where)
            {
                case TO_AFFECTS:
                    sprintf(buf,"Adds %s affect.\n",
                        affect_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_OBJECT:
                    sprintf(buf,"Adds %s object flag.\n",
                        extra_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_IMMUNE:
                    sprintf(buf,"Adds immunity to %s.\n",
                        imm_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_RESIST:
                    sprintf(buf,"Adds resistance to %s.\n\r",
                        res_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_VULN:
                    sprintf(buf,"Adds vulnerability to %s.\n\r",
                        vuln_flags.messages(paf->bitvector).c_str( ));
                    break;
                case TO_DETECTS:
                    sprintf(buf,"Adds %s detection.\n\r",
                        detect_flags.messages(paf->bitvector).c_str( ));
                    break;
                default:
                    sprintf(buf,"Unknown bit %d: %d\n\r",
                        paf->where,paf->bitvector);
                    break;
            }
            ch->send_to(buf);
        }
    }

        sprintf(buf,"Damage condition : %d (%s) ", obj->condition,
        obj->get_cond_alias() );        
        ch->send_to(buf);
        
        if (obj->behavior) {
            ostringstream ostr;
            
            sprintf(buf, "Behavior: [%s]\r\n", obj->behavior->getType( ).c_str( ));
            ch->send_to(buf);

            obj->behavior.toStream( ostr );
            ch->send_to( ostr );
        }

        ch->send_to("\n\r");
        return;
}


void show_char_pk_flags( PCharacter *ch, ostringstream &buf );

static bool has_nochannel(Character *ch)
{
    static const DLString nochannel( "nochannel" );
    
    return !ch->is_npc() && ch->getPC()->getAttributes( ).isAvailable( nochannel );
}

static bool has_nopost(Character *ch)
{
    static const DLString nopost( "nopost" );
    
    return !ch->is_npc() && ch->getPC()->getAttributes( ).isAvailable( nopost );
}


/* NOTCOMMAND */ void do_mstat( Character *ch, Character *victim )
{
    ostringstream buf;
    PCharacter *pc = victim->is_npc( ) ? 0 : victim->getPC( ); // no switched data
    NPCharacter *npc = victim->getNPC( );
    
    buf << "Name: [" << victim->getNameP( ) << "] ";
    if (pc)
        buf << "RName: [" << pc->getRussianName( ).normal( ) << "] ";
    if (npc)
        buf << "Reset Zone: " << (npc->zone ? npc->zone->name : "?");
    buf << endl;
    
    if (npc)
        buf << "Vnum: "   << npc->pIndexData->vnum << "  "
            << "Group: "  << npc->group << "  "
            << "Count: "  << npc->pIndexData->count << "  "
            << "Killed: " << npc->pIndexData->killed
            << endl;

    buf << "Race: " << victim->getRace( )->getName( ) << "  "
        << "Sex: "  << sex_table.name(victim->getSex( )) << "  "
        << "Room: " << victim->in_room->vnum
        << endl;
    
    for (int s = 0; s < stat_table.size; s++)
        buf << stat_table.name(s).capitalize( ) << ": "
            << victim->perm_stat[s] 
            << "(" << victim->getCurrStat(s) << ")"
            << "  ";
    buf << endl;
    
    buf << "Hp: "   << victim->hit << "/" << victim->max_hit << " "
        << "Mana: " << victim->mana << "/" << victim->max_mana << " "
        << "Move: " << victim->move << "/" << victim->max_move << " ";
    if (pc)
        buf << "Prac: "  << pc->practice << " "
            << "Train: " << pc->train << " ";
    buf << endl;
    
    if (victim->getReligion( ) == god_none)
        buf << "Не верит в богов." << endl;
    else
        buf << "Believes the religion of " << victim->getReligion( )->getShortDescr( ) << endl;
    
    buf << "Lev: " << victim->getRealLevel( ) << "(" << victim->getModifyLevel( ) << ")  "
        << "Class: " << victim->getProfession( )->getName( ) << "  "
        << "Align: " << align_table.name(ALIGNMENT(victim)) << " (" << victim->alignment << ")  "
        << "Ethos: " << ethos_table.name( victim->ethos ) << "  "
        << endl;

    buf << "Gold: " << victim->gold << "  "
        << "Silver: " << victim->silver << "  ";
    if (pc)
        buf << "Bank Gold: " << pc->bank_g << "  "
            << "Bank Silver: " << pc->bank_s << "  "
            << "QuestPoints: " << pc->questpoints 
            << endl
            << "Exp: " << pc->exp << "  "
            << "Exp to level: " << pc->getExpToLevel( ) << "  "
            << "Exp per level: " << pc->getExpPerLevel( pc->getLevel( ) + 1 ) - pc->getExpPerLevel( );
    buf << endl;
        
    buf << "Armor: ";
    for (int a = 0; a < ac_type.size; a++)
        buf << ac_type.name(a) << ": " << GET_AC(victim, a) << "  ";
    buf << endl;
    
    buf << "Hit: "   << victim->hitroll << "  "
        << "Dam: "   << victim->damroll << "  "
        << "Saves: " << victim->saving_throw << "  "
        << "Size: "  << size_table.name(victim->size) << " (" << victim->size << ")  "
        << "Pos: "   << position_table.name(victim->position) << "  ";
    if (pc)
        buf << "Wimpy: " << victim->wimpy;
    buf << endl;

    if (npc)
        buf << "Damage: " 
            << npc->damage[DICE_NUMBER] << "d" 
            << npc->damage[DICE_TYPE] << "+"
            << npc->damage[DICE_BONUS] << "  "
            << "Message: " << weapon_flags.name(npc->dam_type)
            << endl;
    
    buf << "Fighting: " << (victim->fighting ? victim->fighting->getNameP( ) : "(none)") << "  ";
    if (pc)
        buf << "Death: " << pc->death << "  ";
    buf << "Carry number: " << victim->carry_number << "  "
        << "Carry weight: " << victim->getCarryWeight( ) / 10
        << endl;
    
        
    if (pc) {
        for (int i = 0; i < desireManager->size( ); i++)
            buf << desireManager->find( i )->getName( )
                << ": " << pc->desires[i] << "  ";
        
        buf << endl
            << "Age: "        << pc->age.getYears( ) << "  "
            << "Played: "     << pc->age.getHours( ) << "(" << pc->age.getTrueHours( ) << ")  "
            << "Last Level: " << pc->last_level << "  "
            << "Timer: "      << pc->timer
            << endl;
    }
    
    if (npc)
        buf << "Act: " << act_flags.names(victim->act) << endl;
        
    if (pc) {
        buf << "Act: " << plr_flags.names(victim->act) << " ";
        show_char_pk_flags( pc, buf );
        buf << endl;
    }
    
    if (pc || victim->comm) {
        buf << "Comm: " 
            << comm_flags.names(victim->comm) << " "
            << add_comm_flags.names(victim->add_comm) << " ";
        
        if (has_nochannel(victim))
            buf << "nochannel ";
        if (has_nopost(victim))
            buf << "nopost ";
        buf << endl;
    }
    
    if (npc && npc->off_flags)
        buf << "Offense: " << off_flags.names(npc->off_flags) << endl;

    if (victim->imm_flags)
        buf << "Immune: " << imm_flags.names(victim->imm_flags) << endl;

    if (victim->res_flags)
        buf << "Resist: " <<  res_flags.names(victim->res_flags) << endl;

    if (victim->vuln_flags)
        buf << "Vulnerable: " << vuln_flags.names(victim->vuln_flags) << endl;

    if (victim->detection)
        buf << "Detection: " <<  detect_flags.names(victim->detection) << endl;
    
    buf << "Form:  " << form_flags.names(victim->form) << "  " << endl
        << "Parts: " << part_flags.names(victim->parts) << endl;
    
    if (victim->affected_by)
        buf << "Affected by " << affect_flags.names(victim->affected_by) << endl;
    
    buf << "Master " <<  (victim->master ? victim->master->getNameP( ) : "(none)") << "  "
        << "Leader " <<  (victim->leader ? victim->leader->getNameP( ) : "(none)") << "  ";
    if (pc)
        buf << "Pet: " << (pc->pet ? pc->pet->getNameP( ) : "(none)");
    buf << endl;
    
    if (npc) {
        buf << "Short description: " << npc->getShortDescr( ) << endl
            << "Long description: "  << npc->getLongDescr( );

        const char *spec_fun_name = spec_name(*npc->spec_fun);
        if (spec_fun_name != 0)
            buf << "Mobile has special procedure " << spec_fun_name << "." << endl;
    }
    
    for (Affect *paf = victim->affected; paf; paf = paf->next) {
        buf << "Affect: '" << paf->type->getName( ) << "', ";

        if (paf->location != APPLY_NONE)
            buf << "modifies " << apply_flags.name( paf->location ) << " "
                << "by " << paf->modifier << ", ";
                
        if (paf->bitvector != 0) {
            const FlagTable *table = 0;
            switch(paf->where) {
            case TO_AFFECTS: table = &affect_flags; break;
            case TO_IMMUNE: 
            case TO_RESIST:
            case TO_VULN:    table = &imm_flags; break;
            case TO_DETECTS: table = &detect_flags; break;
            }

            if (table)
                buf << "adds '" << table->names( paf->bitvector ) << "' "
                    << "to " << affwhere_flags.name( paf->where ) << ", ";
        }

        if (!paf->global.empty( )) {
            switch(paf->where) {
            case TO_LIQUIDS:   buf << "smell of " << paf->global.toString( ) << ", "; break;
            case TO_LOCATIONS: buf << "no rib "   << paf->global.toString( ) << ", "; break;
            }
        }
             
        
        buf << "for " << paf->duration << " hours, level " << paf->level << endl;
    }

        
    if (pc) {
        if (pc->getLastAccessTime( ).getTime( ) != 0)
            buf << "Last played time: " << pc->getLastAccessTime( ).getTimeAsString( ) << endl;
        
        if (!pc->getLastAccessHost( ).empty( ))
            buf << "Last played host: " << pc->getLastAccessHost( ) << endl;
    }

    buf << "Last fought: " << (victim->last_fought ? victim->last_fought->getNameP( ) :"none") << "  "
        << "Last fight time: " << Date::getTimeAsString( victim->getLastFightTime( ) ) 
        << endl;
    
    if (victim->ambushing[0])
        buf << "Ambushing: [" << victim->ambushing << "]" << endl;
    
    if (npc && !npc->pIndexData->practicer.empty( ))
        buf << "Practicer: " << npc->pIndexData->practicer.toString( ) << endl;

    if (npc && npc->behavior) { 
        buf << "Behavior: [" << npc->behavior->getType( ) << "]" << endl;
        npc->behavior.toStream( buf );
    }
    
    page_to_char( buf.str( ).c_str( ), ch );
}

CMDWIZP( vnum )
{
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        ch->send_to("Syntax:\n\r");
        ch->send_to("  vnum obj <name>\n\r");
        ch->send_to("  vnum mob <name>\n\r");
        ch->send_to("  vnum type <item_type>\n\r");
        return;
    }
    
    if (!str_cmp(arg, "type")) {
        do_tfind(ch, string);
        return;
    }

    if (!str_cmp(arg,"obj"))
    {
        do_ofind(ch,string);
         return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
        do_mfind(ch,string);
        return;
    }

    /* do both */
    do_mfind(ch,argument);
    do_ofind(ch,argument);
}


/* NOTCOMMAND */ void do_mfind( Character *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->send_to("Find whom?\n\r");
        return;
    }

    fAll        = false; /* !str_cmp( arg, "all" ); */
    found        = false;
    nMatch        = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != 0 )
        {
            nMatch++;
            if ( fAll || is_name( argument, pMobIndex->player_name ) )
            {
                found = true;
                sprintf( buf, "[%5d] %s\n\r",
                    pMobIndex->vnum, 
                    russian_case( pMobIndex->short_descr, '1' ).c_str( ) );
                ch->send_to(buf);
            }
        }
    }

    if ( !found )
        ch->send_to("No mobiles by that name.\n\r");

    return;
}



/* NOTCOMMAND */ void do_ofind( Character *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        ch->send_to("Find what?\n\r");
        return;
    }

    fAll        = false; /* !str_cmp( arg, "all" ); */
    found        = false;
    nMatch        = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != 0 )
        {
            nMatch++;
            if ( fAll || is_name( argument, pObjIndex->name ) )
            {
                found = true;
                sprintf( buf, "[%5d] %s\n\r",
                    pObjIndex->vnum, 
                    russian_case( pObjIndex->short_descr, '1' ).c_str( ) );
                ch->send_to(buf);
            }
        }
    }

    if ( !found )
        ch->send_to("No objects by that name.\n\r");

    return;
}

/* NOTCOMMAND */ void do_tfind( Character *ch, char *argument )
{
    int type;
    ostringstream buf;
    
    if ((type = item_table.value( argument )) == NO_FLAG) {
        ch->println( "No such item type exists." );
        return;
    }
    
    for (int i=0; i<MAX_KEY_HASH; i++)
        for(OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next) 
            if (pObj->item_type == type) {
                buf << dlprintf( "[%5d] %s\n",
                                 pObj->vnum,
                                 russian_case( pObj->short_descr, '1' ).c_str( ) );
            }

    if (buf.str( ).empty( ))
        ch->println( "No object has such item type." );
    else
        page_to_char( buf.str( ).c_str( ), ch ); 
}


CMDWIZP( owhere )
{
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found = false;;
    int number = 0, max_found = 200;
    int vnum = -1;

    if ( argument[0] == '\0' )
    {
            ch->send_to("Find what?\n\r");
            return;
    }

    if (is_number( argument ))
        vnum = atoi( argument );

    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if (!ch->can_see( obj ))
            continue;
        if (ch->get_trust( ) < obj->level)
            continue;

        if (vnum > 0) {
            if (obj->pIndexData->vnum != vnum)
                continue;
        }
        else {
            if (!is_name( argument, obj->getName( ) ))
                continue;
        }


        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
                ;

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by)
                && in_obj->carried_by->in_room != 0 )
                sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                        number,
                        obj->getShortDescr( '1' ).c_str( ),
                        ch->sees(in_obj->carried_by, '5').c_str(),
                        in_obj->carried_by->in_room->vnum );
        else if ( in_obj->in_room != 0 && ch->can_see(in_obj->in_room) )
                sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                        number,
                        obj->getShortDescr( '1' ).c_str( ),
                        in_obj->in_room->name,
                        in_obj->in_room->vnum );
        else
                sprintf( buf, "%3d) %s is somewhere\n\r",
                        number,
                        obj->getShortDescr( '1' ).c_str( ) );

        buf[0] = Char::upper(buf[0]);
        buffer << buf;

        if ( number >= max_found )
            break;
    }

    if (!found)
        ch->send_to("Nothing like that in heaven or earth.\n\r");
    else
        page_to_char( buffer.str( ).c_str( ), ch );
}


CMDWIZP( mwhere )
{
    char buf[MAX_STRING_LENGTH];
    ostringstream buffer;
    Character *victim;
    bool found;
    int count = 0;
    int vnum = -1;

    if ( argument[0] == '\0' )
    {
            Descriptor *d;

            /* show characters logged */

            for (d = descriptor_list; d != 0; d = d->next)
            {
                if(!d->character)
                    continue;

                if(d->connected != CON_PLAYING)
                    continue;
                
                if(!d->character->in_room)
                    continue;
                
                if(!ch->can_see(d->character))
                    continue;
                
                if(!ch->can_see(d->character->in_room))
                    continue;
                
                victim = d->character;
                count++;

                if (victim->is_npc( ))
                    sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
                            count, 
                            victim->getPC( )->getNameP( ),
                            victim->getNameP('1').c_str( ),
                            victim->in_room->name, victim->in_room->vnum);
                else
                    sprintf(buf,"%3d) %s is in %s [%d]\n\r",
                            count, 
                            victim->getNameP( ),
                            victim->in_room->name, victim->in_room->vnum);

                buffer << buf;
            }

            page_to_char(buffer.str( ).c_str( ), ch);
            return;
    }

    found = false;

    if (is_number( argument ))
        vnum = atoi( argument );

    for (victim = char_list; victim != 0; victim = victim->next)
    {
        if (vnum > 0) {
            if (!victim->is_npc( ))
                continue;
            if (victim->getNPC( )->pIndexData->vnum != vnum)
                continue;
        } 
        else {
            if (!is_name( argument, victim->getNameP( '7' ).c_str( ) ))
                continue;
        }

        found = true;
        count++;
        sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                victim->is_npc() ? victim->getNPC()->pIndexData->vnum : 0,
                victim->is_npc() ? victim->getNameP( '1' ).c_str( ) : victim->getNameP( ),
                victim->in_room->vnum,
                victim->in_room->name );
        buffer << buf;
    }

    if (!found)
        act_p( "You didn't find any $T.", ch, 0, argument, TO_CHAR,POS_DEAD );
    else
        page_to_char(buffer.str( ).c_str( ),ch);

}






CMDWIZP( shutdown )
{
    Descriptor *d,*d_next;
    
    DLFileAppend( dreamland->getBasePath( ), dreamland->getShutdownFile( ) )
         .printf( "Shutdown %s by %s\n",
                    Date::getCurrentTimeAsString( ).c_str( ),
                    ch->getNameP( )
                );

    /* TODO save all */
    dreamland->shutdown( );

    for ( d = descriptor_list; d != 0; d = d_next)
    {
        d_next = d->next;
        d->close( );
    }
}

CMDWIZP( protect )
{
    Character *victim;

    if (argument[0] == '\0')
    {
        ch->send_to("Protect whom from snooping?\n\r");
        return;
    }

    if ((victim = get_char_world(ch,argument)) == 0)
    {
        ch->send_to("You can't find them.\n\r");
        return;
    }

    if (IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
        act_p("$C1 is no longer snoop-proof.",ch,0,victim,TO_CHAR,POS_DEAD);
        victim->send_to("Your snoop-proofing was just removed.\n\r");
        victim->comm.removeBit(COMM_SNOOP_PROOF);
    }
    else
    {
        act_p("$C1 is now snoop-proof.",ch,0,victim,TO_CHAR,POS_DEAD);
        victim->send_to("You are now immune to snooping.\n\r");
        victim->comm.setBit(COMM_SNOOP_PROOF);
    }
}



CMDWIZP( snoop )
{
    char arg[MAX_INPUT_LENGTH];
    Descriptor *d;
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Snoop whom?\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->desc == 0 )
    {
        ch->send_to("No descriptor to snoop.\n\r");
        return;
    }

    if ( victim == ch )
    {
        ch->send_to("Cancelling all snoops.\n\r");
        wiznet( WIZ_SNOOPS, WIZ_SECURE, ch->get_trust(), "%C1 stops being such a snoop.", ch );
        for ( d = descriptor_list; d != 0; d = d->next )
        {
            if ( d->snoop_by == ch->desc )
                d->snoop_by = 0;
        }
        return;
    }

    if ( victim->desc->snoop_by != 0 )
    {
        ch->send_to("Busy already.\n\r");
        return;
    }

    if (!victim->in_room->isOwner(ch) && ch->in_room != victim->in_room
    &&  victim->in_room->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->send_to("That character is in a private room.\n\r");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust()
    ||   IS_SET(victim->comm,COMM_SNOOP_PROOF))
    {
        ch->send_to("You failed.\n\r");
        return;
    }

    if ( ch->desc != 0 ) {
        for ( d = ch->desc->snoop_by; d != 0; d = d->snoop_by ) {
            if ( d->character == victim || d->character->getPC( ) == victim ) {
                ch->send_to("Snoop loop detected.\n\r");
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    wiznet( WIZ_SNOOPS, WIZ_SECURE, ch->get_trust(), "%C1 starts snooping on %C1.", ch, victim );
    ch->send_to("Ok.\n\r");
}



CMDWIZP( switch )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if(ch->is_npc( )) {
        ch->send_to("Mobs can't switch.\n\r");
        return;
    }
    
    one_argument( argument, arg );

    PCharacter *pch = ch->getPC( );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Switch into whom?\n\r");
        return;
    }

    if ( ch->desc == 0 )
        return;

    if ( pch->switchedTo )
    {
        ch->send_to("You are already switched.\n\r");
        return;
    }
    
    victim = get_char_world( ch, arg );
    if ( !victim )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim == ch ) {
        ch->send_to("Ok.\n\r");
        return;
    }

    if (!victim->is_npc()) {
        ch->send_to("You can only switch into mobiles.\n\r");
        return;
    }

    NPCharacter *mob = victim->getNPC( );

    if (ch->in_room != victim->in_room &&  victim->in_room->isPrivate( ) &&
            !IS_TRUSTED(ch,IMPLEMENTOR))
    {
        ch->send_to("That character is in a private room.\n\r");
        return;
    }

    if ( victim->desc != 0 ) {
        ch->send_to("Character in use.\n\r");
        return;
    }

    wiznet( WIZ_SWITCHES, WIZ_SECURE, ch->get_trust(), "%C1 switches into %C4.", pch, mob );

    mob->switchedFrom = pch;
    pch->switchedTo = mob;
    
    ch->desc->associate( victim );
    ch->desc = 0;
    
    /* change communications to match */
    victim->prompt = ch->prompt;
    victim->comm = ch->comm;
    victim->lines = ch->lines;
    victim->send_to("Ok.\n\r");
    return;
}


static bool mprog_return( Character *ch )
{
    FENIA_CALL( ch, "Return", "" );
    FENIA_NDX_CALL( ch->getNPC( ), "Return", "C", ch );
    return false;
}

CMDWIZP( return )
{
    NPCharacter *mob = ch->getNPC();

    if(!mob) {
        ch->println("Ты и так в своем теле.");
        return;
    }
    
    if ( mob->desc == 0 )
        return;
    
    if ( !mob->switchedFrom ) {
        ch->send_to("You aren't switched.\n\r");
        return;
    }

    if (mprog_return( mob ))
        return;

    mob->send_to("You return to your original body. Type replay to see any missed tells.\n\r");
    ch->prompt.clear( );

    wiznet( WIZ_SWITCHES, WIZ_SECURE, ch->get_trust( ), 
            "%C1 returns from %C2.", mob->switchedFrom, mob );
    
    mob->desc->associate( mob->switchedFrom );
    mob->switchedFrom->switchedTo = 0;
    mob->switchedFrom = 0;
    mob->desc = 0;
}

/* trust levels for load and clone */
bool obj_check (Character *ch, Object *obj)
{
/*    if (IS_TRUSTED(ch,GOD)
        || (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
        || (IS_TRUSTED(ch,DEMI)            && obj->level <= 10 && obj->cost <= 500)
        || (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
        || (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
        return true;
    else
        return false;*/
    //by razer - nonense check        
    return true;        
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(Character *ch, Object *obj, Object *clone)
{
        Object *c_obj, *t_obj;

        for (c_obj = obj->contains; c_obj != 0; c_obj = c_obj->next_content)
        {
                if (obj_check(ch,c_obj))
                {
                        t_obj = create_object(c_obj->pIndexData,0);
                        clone_object(c_obj,t_obj);
                        obj_to_obj(t_obj,clone);
                        recursive_clone(ch,c_obj,t_obj);
                }
        }
}

/* command that is similar to load */
CMDWIZP( clone )
{
        char arg[MAX_INPUT_LENGTH];
        char *rest;
        Character *mob;
        Object  *obj;

        rest = one_argument(argument,arg);

        if (arg[0] == '\0')
        {
                ch->send_to("Clone what?\n\r");
                return;
        }

        if (!str_prefix(arg,"object"))
        {
                mob = 0;
                obj = get_obj_here(ch,rest);
                if (obj == 0)
                {
                        ch->send_to("You don't see that here.\n\r");
                        return;
                }
        }
        else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character"))
        {
                obj = 0;
                mob = get_char_room(ch,rest);
                if (mob == 0)
                {
                        ch->send_to("You don't see that here.\n\r");
                        return;
                }
        }
        else /* find both */
        {
                mob = get_char_room(ch,argument);
                obj = get_obj_here(ch,argument);
                if (mob == 0 && obj == 0)
                {
                        ch->send_to("You don't see that here.\n\r");
                        return;
                }
        }

        /* clone an object */
        if (obj != 0)
        {
                Object *clone;

                if (!obj_check(ch,obj))
                {
                        ch->send_to("Your powers are not great enough for such a task.\n\r");
                        return;
                }

                clone = create_object(obj->pIndexData,0);
                clone_object(obj,clone);

                if (obj->carried_by != 0)
                        obj_to_char(clone,ch);
                else
                        obj_to_room(clone,ch->in_room);

                recursive_clone(ch,obj,clone);

                act_p("$c1 создает $o4.",ch,clone,0,TO_ROOM,POS_RESTING);
                act_p("Ты создаешь дубликат $o2.",ch,clone,0,TO_CHAR,POS_RESTING);
                wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), 
                        "%C1 клонирует %O4.", ch, obj );
                return;
        }
        else if (mob != 0)
        {
                NPCharacter *clone;
                Object *new_obj;

                if (!mob->is_npc())
                {
                        ch->send_to("You can only clone mobiles.\n\r");
                        return;
                }

                if ( (mob->getRealLevel( ) > 20 && !IS_TRUSTED(ch,GOD))
                        || (mob->getRealLevel( ) > 10 && !IS_TRUSTED(ch,IMMORTAL))
                        || (mob->getRealLevel( ) >  5 && !IS_TRUSTED(ch,DEMI))
                        || (mob->getRealLevel( ) >  0 && !IS_TRUSTED(ch,ANGEL))
                        || !IS_TRUSTED(ch,AVATAR) )
                {
                        ch->send_to("Your powers are not great enough for such a task.\n\r");
                        return;
                }

                clone = create_mobile(mob->getNPC()->pIndexData);
                clone_mobile(mob->getNPC(),clone);
        
                for (obj = mob->carrying; obj != 0; obj = obj->next_content)
                {
                        if (obj_check(ch,obj))
                        {
                                new_obj = create_object(obj->pIndexData,0);
                                clone_object(obj,new_obj);
                                recursive_clone(ch,obj,new_obj);
                                obj_to_char(new_obj,clone);
                                new_obj->wear_loc = obj->wear_loc;
                        }
                }

                char_to_room(clone,ch->in_room);
                act_p("$c1 создает $C4.",ch,0,clone,TO_ROOM,POS_RESTING);
                act_p("Ты клонируешь $C4.",ch,0,clone,TO_CHAR,POS_RESTING);
                wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), 
                        "%C1 клонирует %C4.", ch, clone );
                
                return;
        }
}

/* RT to replace the two load commands */

CMDWIZP( load )
{
   char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        ch->send_to("Syntax:\n\r");
        ch->send_to("  load mob <vnum>\n\r");
        ch->send_to("  load obj <vnum> <level>\n\r");
        return;
    }

    if (!str_cmp(arg,"mob") || !str_cmp(arg,"char"))
    {
        do_mload(ch,argument);
        return;
    }

    if (!str_cmp(arg,"obj"))
    {
        do_oload(ch,argument);
        return;
    }
    /* echo syntax */
    run(ch, str_empty);
}


/* NOTCOMMAND */ void do_mload( Character *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        MOB_INDEX_DATA *pMobIndex;
        Character *victim;

        one_argument( argument, arg );

        if ( arg[0] == '\0' || !is_number(arg) )
        {
                ch->send_to("Syntax: load mob <vnum>.\n\r");
                return;
        }

        if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == 0 )
        {
                ch->send_to("No mob has that vnum.\n\r");
                return;
        }

        victim = create_mobile( pMobIndex );

        if (victim->in_room == 0)
            char_to_room( victim, ch->in_room );

        act_p( "$c1 создает $C4!", ch, 0, victim, TO_ROOM,POS_RESTING );
        act_p( "Ты создаешь $C4!", ch, 0, victim, TO_CHAR,POS_RESTING );


        wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust(), 
                "%C1 создает %C4.", ch, victim );
        ch->send_to("Ok.\n\r");
        return;
}



/* NOTCOMMAND */ void do_oload( Character *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    Object *obj;
    short level;

    argument = one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || !is_number(arg1))
    {
        ch->send_to("Syntax: load obj <vnum> <level>.\n\r");
        return;
    }

    level = ch->get_trust(); /* default */

    if ( arg2[0] != '\0')  /* load with a level */
    {
        if (!is_number(arg2))
        {
          ch->send_to("Syntax: oload <vnum> <level>.\n\r");
          return;
        }
        level = atoi(arg2);
        if (level < 0 || level > ch->get_trust())
        {
          ch->send_to("Level must be be between 0 and your level.\n\r");
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == 0 )
    {
        ch->send_to("No object has that vnum.\n\r");
        return;
    }

    obj = create_object( pObjIndex, level );
    if ( obj->can_wear( ITEM_TAKE) )
        obj_to_char( obj, ch );
    else
        obj_to_room( obj, ch->in_room );
        
    act_p( "$c1 создает $o4!", ch, obj, 0, TO_ROOM,POS_RESTING );
    act_p( "Ты создаешь $o4!", ch, obj, 0, TO_CHAR,POS_RESTING );
    wiznet( WIZ_LOAD, WIZ_SECURE, ch->get_trust( ), "%C1 loads %O4.", ch, obj );
    
    LogStream::sendNotice( ) 
        << ch->getName( ) << " loads obj vnum " << obj->pIndexData->vnum
        << " id " << obj->getID( ) << endl;

    return;
}



CMDWIZP( purge )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        Character *vnext;
        Object  *obj_next;
        
        dreamland->removeOption( DL_SAVE_MOBS );
        dreamland->removeOption( DL_SAVE_OBJS );

        for ( victim = ch->in_room->people; victim != 0; victim = vnext )
        {
            vnext = victim->next_in_room;
            if ( victim->is_npc() && !IS_SET(victim->act,ACT_NOPURGE)
            &&   victim != ch /* safety precaution */ )
                extract_char( victim );
        }

        for ( obj = ch->in_room->contents; obj != 0; obj = obj_next )
        {
            obj_next = obj->next_content;
            if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
              extract_obj( obj );
        }

        act_p( "$c1 purges the room!", ch, 0, 0, TO_ROOM,POS_RESTING);
        ch->send_to("Ok.\n\r");
        dreamland->resetOption( DL_SAVE_MOBS );
        dreamland->resetOption( DL_SAVE_OBJS );
        save_items( ch->in_room );
        save_mobs( ch->in_room );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( !victim->is_npc() )
    {
          ch->send_to("Maybe that wasn't a good idea...\n\r");
          return;
    }

    act_p( "$c1 purges $C4.", ch, 0, victim, TO_NOTVICT,POS_RESTING );
    extract_char( victim );
}





CMDWIZP( restore )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Character *vch;
    Descriptor *d;

    one_argument( argument, arg );
    if (arg[0] == '\0' || !str_cmp(arg,"room"))
    {
    /* cure room */
            
        for (vch = ch->in_room->people; vch != 0; vch = vch->next_in_room)
        {
            affect_strip(vch,gsn_plague);
            affect_strip(vch,gsn_poison);
            affect_strip(vch,gsn_blindness);
            affect_strip(vch,gsn_sleep);
            affect_strip(vch,gsn_curse);

            vch->hit         = vch->max_hit;
            vch->mana        = vch->max_mana;
            vch->move        = vch->max_move;
            update_pos( vch);
            act_p("$c1 has restored you.",ch,0,vch,TO_VICT,POS_DEAD);
        }

        wiznet( WIZ_RESTORE, WIZ_SECURE, ch->get_trust(), 
                "%C1 restored room %d.", ch, ch->in_room->vnum );

        ch->send_to("Room restored.\n\r");
        return;

    }

    if ( ch->get_trust() >=  MAX_LEVEL - 1 && !str_cmp(arg,"all"))
    {
    /* cure all */
            
        for (d = descriptor_list; d != 0; d = d->next)
        {
            if (d->connected != CON_PLAYING)
                continue;

            victim = d->character;

            if (victim == 0 || victim->is_npc())
                continue;

            affect_strip(victim,gsn_plague);
            affect_strip(victim,gsn_poison);
            affect_strip(victim,gsn_blindness);
            affect_strip(victim,gsn_sleep);
            affect_strip(victim,gsn_curse);

            victim->hit         = victim->max_hit;
            victim->mana        = victim->max_mana;
            victim->move        = victim->max_move;
            update_pos( victim);
            if (victim->in_room != 0)
                act_p("$c1 has restored you.",ch,0,victim,TO_VICT,POS_DEAD);
        }
        ch->send_to("All active players restored.\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    affect_strip(victim,gsn_plague);
    affect_strip(victim,gsn_poison);
    affect_strip(victim,gsn_blindness);
    affect_strip(victim,gsn_sleep);
    affect_strip(victim,gsn_curse);
    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos( victim );

    act_p( "$c1 has restored you.", ch, 0, victim, TO_VICT,POS_DEAD );
    wiznet( WIZ_RESTORE, WIZ_SECURE, ch->get_trust( ), "%C1 restored %C4.", ch, victim );
    ch->send_to("Ok.\n\r");
}

         
CMDWIZP( freeze )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Freeze whom?\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->send_to("Not on NPC's.\n\r");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->send_to("You failed.\n\r");
        return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
        victim->act.removeBit( PLR_FREEZE);
        victim->send_to("You can play again.\n\r");
        ch->send_to("FREEZE removed.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 thaws %C4.", ch, victim );
    }
    else
    {
        victim->act.setBit( PLR_FREEZE);
        victim->send_to("You can't do ANYthing!\n\r");
        ch->send_to("FREEZE set.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, 
                "%C1 puts %C4 in the deep freeze.", ch, victim );
    }

    if( !victim->is_npc( ) ) 
        victim->getPC( )->save();
}



CMDWIZP( log )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Log whom?\n\r");
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if (!dreamland->hasOption( DL_LOG_ALL )) {
            dreamland->setOption( DL_LOG_ALL );
            ch->send_to("Log ALL on.\n\r");
        }
        else {
            dreamland->removeOption( DL_LOG_ALL );
            ch->send_to("Log ALL off.\n\r");
        }

        return;
    }

    if (!str_cmp( arg, "imm" )) {
        if (!dreamland->hasOption( DL_LOG_IMM )) {
            dreamland->setOption( DL_LOG_IMM );
            ch->send_to( "Immortal logging is now ON.\r\n" );
        }
        else {
            dreamland->removeOption( DL_LOG_IMM );
            ch->send_to( "Immortal logging is now OFF.\r\n" );
        }

        return;
    }
    
    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->send_to("Not on NPC's.\n\r");
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
        victim->act.removeBit( PLR_LOG);
        ch->send_to("LOG removed.\n\r");
    }
    else
    {
        victim->act.setBit( PLR_LOG);
        ch->send_to("LOG set.\n\r");
    }

    return;
}



CMDWIZP( noemote )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Noemote whom?\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }


    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->send_to("You failed.\n\r");
        return;
    }

    if ( IS_SET(victim->comm, COMM_NOEMOTE) )
    {
        victim->comm.removeBit( COMM_NOEMOTE);
        victim->send_to("You can emote again.\n\r");
        ch->send_to("NOEMOTE removed.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 restores emotes to %C1.", ch, victim );
    }
    else
    {
        victim->comm.setBit( COMM_NOEMOTE);
        victim->send_to("You can't emote!\n\r");
        ch->send_to("NOEMOTE set.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 revokes %C2 emotes.", ch, victim );
    }
}


CMDWIZP( notell )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->send_to("Notell whom?");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == 0 )
    {
        ch->send_to("They aren't here.\n\r");
        return;
    }

    if ( victim->get_trust() >= ch->get_trust() )
    {
        ch->send_to("You failed.\n\r");
        return;
    }

    if ( IS_SET(victim->comm, COMM_NOTELL) )
    {
        victim->comm.removeBit( COMM_NOTELL);
        victim->send_to("You can tell again.\n\r");
        ch->send_to("NOTELL removed.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 restores tells to %C2.", ch, victim );
    }
    else
    {
        victim->comm.setBit( COMM_NOTELL);
        victim->send_to("You can't tell!\n\r");
        ch->send_to("NOTELL set.\n\r");
        wiznet( WIZ_PENALTIES, WIZ_SECURE, 0, "%C1 revokes %C1 tells.", ch, victim );
    }
}



CMDWIZP( peace )
{
    Character *rch;

    for ( rch = ch->in_room->people; rch != 0; rch = rch->next_in_room )
    {
        if ( rch->fighting != 0 )
            stop_fighting( rch, true );
        if (rch->is_npc() && IS_SET(rch->act,ACT_AGGRESSIVE))
            rch->act.removeBit(ACT_AGGRESSIVE);
    }

    ch->send_to("Ok.\n\r");
    return;
}

CMDWIZP( wizlock )
{
    bitstring_t opt = DL_WIZLOCK;

    if (dreamland->hasOption( opt ))
    {
        dreamland->removeOption( opt );
        wiznet( 0, 0, 0, "%C1 removes wizlock.", ch );
        ch->send_to("Game un-wizlocked.\n\r");
    }
    else
    {
        dreamland->setOption( opt );
        wiznet( 0, 0, 0, "%C1 has wizlocked the game.", ch );
        ch->send_to("Game wizlocked.\n\r");
    }
}

/* RT anti-newbie code */

CMDWIZP( newlock )
{
    bitstring_t opt = DL_NEWLOCK;

    if (dreamland->hasOption( opt ))
    {
        dreamland->removeOption( opt );
        wiznet( 0, 0, 0, "%C1 allows new characters back in.", ch );
        ch->send_to("Newlock removed.\n\r");
    }
    else
    {
        dreamland->setOption( opt );
        wiznet( 0, 0, 0, "%C1 locks out new characters.", ch );
        ch->send_to("New characters have been locked out.\n\r");
    }
}





CMDWIZP( string )
{
        char type [MAX_INPUT_LENGTH];
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        Character *victim;
        Object *obj;

        argument = one_argument( argument, type );
        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );

        if ( type[0] == '\0'
                || arg1[0] == '\0'
                || arg2[0] == '\0'
                || arg3[0] == '\0' )
        {
                ch->send_to("Syntax:\n\r");
                ch->send_to("  string mob <name> <field> <string>\n\r");
                ch->send_to("    fields: name short long desc title spec\n\r");
                ch->send_to("  string obj  <name> <field> <string>\n\r");
                ch->send_to("    fields: name short long\n\r");
                ch->send_to("  string obj  <name> ed <add|remove|clear> <keyword> <string>\n\r");
                return;
        }

        if ( !str_prefix(type,"character")
                || !str_prefix(type,"mobile"))
        {
                if ( ( victim = get_char_world( ch, arg1 ) ) == 0 )
                {
                        ch->send_to("They aren't here.\n\r");
                        return;
                }

                /* clear zone for mobs */
                if (victim->is_npc())
                    victim->getNPC()->zone = 0;

                /* string something */

                if ( !str_prefix( arg2, "name" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }

                        DLString name( arg3 );
                        victim->setName( name );
                        return;
                }
            
                if ( !str_prefix( arg2, "description" ) )
                {
                        victim->setDescription( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "short" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }
                        victim->getNPC()->setShortDescr( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "long" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }
                        strcat(arg3,"\n\r");
                        victim->getNPC()->setLongDescr( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "title" ) )
                {
                        if ( victim->is_npc() )
                        {
                                ch->send_to("Not on NPC's.\n\r");
                                return;
                        }
                        
                        victim->getPC( )->setTitle( arg3 );
                        return;
                }

                if ( !str_prefix( arg2, "spec" ) )
                {
                        if ( !victim->is_npc() )
                        {
                                ch->send_to("Not on PC's.\n\r");
                                return;
                        }

                        if ( ( victim->getNPC()->spec_fun = spec_lookup( arg3 ) ) == 0 )
                        {
                                ch->send_to("No such spec fun.\n\r");
                                return;
                        }

                        return;
                }
        }

        if (!str_prefix(type,"object"))
        {
                // string an obj
            
                if ( ( obj = get_obj_world( ch, arg1 ) ) == 0 )
                {
                        ch->send_to("Nothing like that in heaven or earth.\n\r");
                        return;
                }

                if ( obj->pIndexData->limit != -1 )
                {
                        ch->send_to("Хмм.. Мохам будет возмущен, не надо.\n\r");
                        return;
                }
            
                if ( !str_prefix( arg2, "name" ) )
                {
                        obj->setName( arg3 );
                }
                else
                if ( !str_prefix( arg2, "short" ) )
                {
                        obj->setShortDescr( arg3 );
                }
                else
                if ( !str_prefix( arg2, "long" ) )
                {
                        obj->setDescription( arg3 );
                }
                else
                if ( !str_prefix( arg2, "ed" ) || !str_prefix( arg2, "extended"))
                {
                        EXTRA_DESCR_DATA *ed;

                        argument = one_argument( argument, arg3 );
                        if ( argument == 0 )
                        {
                                ch->send_to("Syntax: oset <object> ed <add|remove|clear> <keyword> <string>\n\r");
                                return;
                        }

                        if ( !str_prefix (arg3, "clear") )
                        {
                                EXTRA_DESCR_DATA *ed_next;

                                for (ed = obj->extra_descr; ed != 0; ed = ed_next)
                                {
                                        ed_next = ed->next;
                                        free_extra_descr (ed);
                                }

                                obj->extra_descr = 0;

                        }
                        else if ( !str_prefix (arg3, "add") )
                        {

                                argument = one_argument( argument, arg3 );

                                strcat(argument,"\n\r");

                                ed = new_extra_descr();

                                ed->keyword                = str_dup( arg3     );
                                ed->description        = str_dup( argument );
                                ed->next                = obj->extra_descr;
                                obj->extra_descr        = ed;
                        }
                        else if ( !str_prefix (arg3, "remove") )
                        {
                                EXTRA_DESCR_DATA *ed_next;

                                argument = one_argument( argument, arg3 );

                                for (ed = obj->extra_descr; ed != 0; ed = ed_next)
                                {
                                        ed_next = ed->next;

                                        if ( is_name (arg3, ed->keyword) )
                                        {
                                                if (obj->extra_descr == ed)
                                                {
                                                        obj->extra_descr = ed_next;
                                                }
                                                free_extra_descr (ed);
                                        }
                                }
                        }
                        else
                        {
                                ch->send_to ("А может все таки синтакс проверим, а?\n\r");
                                return;
                        }
                }
                else
                {
                        /* echo bad use message */
                        run(ch, str_empty);
                        return;
                }

                save_items_at_holder( obj );
        }
  else
        {
                /* echo bad use message */
                run(ch, str_empty);
        }
}

int decode_flags(char * arg, int * value_add, int * value_sub)
{
        bool negative = false, additive = false;

        int value = atoi( arg );

        *value_add = 0;
        *value_sub = 0;

        if ( value == 0 )
                for (int i = 0; arg[i] != '\0'; i++ )
                {
                        switch ( arg[i] )
                        {
                        case '|' :
                                break;
                        case '-' :
                                additive = false;
                                negative = true;
                                break;
                        case '+' :
                                negative = false;
                                additive = true;
                                break;
                        default :
                                if ( ('A' <= arg[i] && arg[i] <= 'Z')
                                        || ('a' <= arg[i] && arg[i] <= 'z') )
                                {
                                        if ( additive )
                                                SET_BIT( *value_add, flag_convert( arg[i] ) );
                                        else if ( negative )
                                                SET_BIT( *value_sub, flag_convert( arg[i] ) );
                                        else
                                                SET_BIT( value, flag_convert( arg[i] ) );
                                }
                        }
                }

        return value;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
CMDWIZP( force )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        ch->send_to("Force whom to do what?\n\r");
        return;
    }

    one_argument(argument,arg2);

    if (!str_cmp(arg2,"delete"))
    {
        ch->send_to("That will NOT be done.\n\r");
        return;
    }

    sprintf( buf, "$c1 forces you to '%s'.", argument );

    if ( !str_cmp( arg, "all" ) )
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 3)
        {
            ch->send_to("Not at your level!\n\r");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust() )
            {
                act_p( buf, ch, 0, vch, TO_VICT,POS_DEAD );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"players"))
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 2)
        {
            ch->send_to("Not at your level!\n\r");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust()
            &&         vch->getRealLevel( ) < LEVEL_HERO)
            {
                act_p( buf, ch, 0, vch, TO_VICT,POS_DEAD );
                interpret( vch, argument );
            }
        }
    }
    else if (!str_cmp(arg,"gods"))
    {
        Character *vch;
        Character *vch_next;

        if (ch->get_trust() < MAX_LEVEL - 2)
        {
            ch->send_to("Not at your level!\n\r");
            return;
        }

        for ( vch = char_list; vch != 0; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !vch->is_npc() && vch->get_trust() < ch->get_trust()
            &&   vch->getRealLevel( ) >= LEVEL_HERO)
            {
                act_p( buf, ch, 0, vch, TO_VICT,POS_DEAD );
                interpret( vch, argument );
            }
        }
    }
    else
    {
        Character *victim;

        if ( ( victim = get_char_world( ch, arg ) ) == 0 )
        {
            ch->send_to("They aren't here.\n\r");
            return;
        }

        if ( victim == ch )
        {
            ch->send_to("Aye aye, right away!\n\r");
            return;
        }

            if (!victim->in_room->isOwner(ch)
        &&  ch->in_room != victim->in_room
        &&  victim->in_room->isPrivate( ) && !IS_TRUSTED(ch,IMPLEMENTOR))
            {
            ch->send_to("That character is in a private room.\n\r");
            return;
        }

        if ( victim->get_trust() >= ch->get_trust() )
        {
            ch->send_to("Do it yourself!\n\r");
            return;
        }

        if ( !victim->is_npc() && ch->get_trust() < MAX_LEVEL -3)
        {
            ch->send_to("Not at your level!\n\r");
            return;
        }

        act_p( buf, ch, 0, victim, TO_VICT,POS_DEAD );
        interpret( victim, argument );
    }

    ch->send_to("Ok.\n\r");
    return;
}



/*
 * New routines by Dionysos.
 */
CMDWIZP( wizinvis )
{
    short level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( ch->invis_level)
      {
          ch->invis_level = 0;
          act_p( "$c1 slowly fades into existence.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("You slowly fade back into existence.\n\r");
      }
      else
      {
          ch->invis_level = 102;
          act_p( "$c1 slowly fades into thin air.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("You slowly vanish into thin air.\n\r");
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->get_trust())
      {
        ch->send_to("Invis level must be between 2 and your level.\n\r");
        return;
      }
      else
      {
          ch->reply = 0;
          ch->invis_level = level;
          act_p( "$c1 slowly fades into thin air.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("You slowly vanish into thin air.\n\r");
      }
    }

    return;
}


CMDWIZP( incognito )
{
    short level;
    char arg[MAX_STRING_LENGTH];

    /* RT code for taking a level argument */
    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    /* take the default path */

      if ( ch->incog_level)
      {
          ch->incog_level = 0;
          act_p( "$c1 больше не маскируется.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты больше не маскируешься.\n\r");
      }
      else
      {
          ch->incog_level = 102;
          act_p( "$c1 скрывает $s присутствие.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты скрываешь свое присутствие.\n\r");
      }
    else
    /* do the level thing */
    {
      level = atoi(arg);
      if (level < 2 || level > ch->get_trust())
      {
        ch->send_to("Incog level must be between 2 and your level.\n\r");
        return;
      }
      else
      {
          ch->reply = 0;
          ch->incog_level = level;
          act_p( "$c1 скрывает $s присутствие.", ch, 0, 0, TO_ROOM,POS_RESTING );
          ch->send_to("Ты скрываешь свое присутствие.\n\r");
      }
    }

    return;
}




CMDWIZP( advance )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Character *vict;
    PCharacter *victim;
    short level;
    int iLevel;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) )
    {
        ch->send_to("Syntax: advance <char> <level>.\n\r");
        return;
    }

    if ( ( vict = get_char_room( ch, arg1 ) ) == 0 )
    {
        ch->send_to("That player is not here.\n\r");
        return;
    }

    if ( vict->is_npc() )
    {
        ch->send_to("Not on NPC's.\n\r");
        return;
    }

    victim = vict->getPC();
    
    if ( ( level = atoi( arg2 ) ) < 1 || level > 110 )
    {
        ch->send_to("Level must be 1 to 110.\n\r");
        return;
    }

    if ( level > ch->get_trust() )
    {
        ch->send_to("Limited to your trust level.\n\r");
        return;
    }

    /*
     * Lower level:
     *   Reset to level 1.
     *   Then raise again.
     *   Currently, an imp can lower another imp.
     *   -- Swiftest
     */
    if ( level <= victim->getRealLevel( ) )
    {
        int temp_prac, temp_train;

        ch->send_to("Lowering a player's level!\n\r");
        victim->send_to("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r");
        temp_prac = victim->practice;
        temp_train = victim->train;
        victim->setLevel( 1 );
        victim->exp      = victim->getExpToLevel( );
        victim->max_hit  = 10;
        victim->max_mana = 100;
        victim->max_move = 100;
        victim->practice = 0;
        victim->train    = 0;
        victim->max_skill_points = 1000;
        victim->hit      = victim->max_hit;
        victim->mana     = victim->max_mana;
        victim->move     = victim->max_move;
        victim->getPC( )->advanceLevel( );
        victim->practice = temp_prac;
        victim->train    = temp_train;
    }
    else
    {
        ch->send_to("Raising a player's level!\n\r");
        victim->send_to("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r");
    }

    for ( iLevel = victim->getRealLevel( ) ; iLevel < level; iLevel++ )
    {
        if( victim->get_trust() != 0xFFFF )
            victim->send_to("You raise a level!!  ");
        victim->exp += victim->getExpToLevel( );;
        victim->setLevel( victim->getRealLevel( ) + 1 );
        victim->getPC( )->advanceLevel( );
    }
    victim->getPC( )->setTrust( 0 );
    if( !victim->is_npc( ) ) victim->getPC( )->save();
    return;
}



CMDWIZP( smite )
{
  Character *victim;

  if (argument[0] == '\0')
    {
      ch->send_to("От расстройства ты бьешь молнией себя!  Oххх!\n\r");
      return;
    }

  if ((victim = get_char_world(ch, argument)) == 0)
    {
      ch->send_to("Придеться подождать немного и послать на них молнию в другой раз.\n\r");
      return;
    }

  if (victim->is_npc())
    {
      ch->send_to("Этот бедный моб не сделал тебе ничего плохого.\n\r");
      return;
    }

  if (victim->getRealLevel() > ch->getRealLevel())
    {
      ch->send_to("Как ты смеешь!\n\r");
      return;
    }

  if (victim->position < POS_SLEEPING)
    {
      ch->send_to("Грешно издеваться над больными людьми.\n\r");
      return;
    }

  act_p("{CБоги {Rв гневе{C!{/{cОгромная молния, сорвавшаяся с небес, поражает тебя!{/{RЭто было БОЛЬНО! Это было МУЧИТЕЛЬНО БОЛЬНО!{x\n\r", victim, 0,
        ch, TO_CHAR,POS_DEAD);
  act_p("Твоя молния поражает $c4!\n\r", victim, 0, ch, TO_VICT,POS_DEAD);
  act_p("{cОгромная молния, сорвавшаяся с небес, поражает $c4!{x\n\r", victim, 0, ch, TO_NOTVICT,POS_DEAD);
  victim->hit = victim->hit / 2;
  victim->move = victim->move / 2;
  victim->mana = victim->mana / 2;
  return;
}

CMDWIZP( popularity )
{
    ostringstream buf;
    AREA_DATA *area;
    extern AREA_DATA *area_first;
    int i;
    bool fAll;

    fAll = !str_cmp( argument, "all" );
    buf << "Area popularity statistics (in char * ticks)" << endl;

    for (area = area_first, i = 1; area != 0; area = area->next) {
        if (fAll || area->count > 0) {
            buf << fmt( 0, "%-30.30s %-5.5d", 
                           area->name, min( (long unsigned int)0xffff, area->count ) );
            if (i++ % 2 == 0)
                buf << endl;
        }
    }

    buf << endl;
    page_to_char( buf.str( ).c_str( ), ch );
}

CMDWIZP( ititle )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )  {
        ch->send_to("Change whose title to what?\n\r");
        return;
    }

    victim = get_char_world( ch, arg );
    if (victim == 0)  {
        ch->send_to("Nobody is playing with that name.\n\r");
        return;
    }

    if ( victim->is_npc() )
        return;

    if ( argument[0] == '\0' )
    {
        ch->send_to("Change the title to what?\n\r");
        return;
    }
    victim->getPC( )->setTitle( argument );
    ch->send_to("Ok.\n\r");
}

CMDWIZP( rename )
{
    DLString arguments = argument;
    DLString oldName = arguments.getOneArgument( );
    DLString newName = arguments.getOneArgument( );
    DLString& russianName = arguments;

    newName.capitalize( );
    russianName.capitalize( );
    
    if (oldName.empty( )) {
        ch->send_to( "Rename who?\n\r" );
        return;
    }
    
    PCharacter* victim = get_player_world( ch->getPC( ), oldName.c_str( ) );
    
    if (!victim) {
        ch->send_to("There is no such a person online.\n\r");
        return;
    }
    
    if( (victim != ch ) && ( victim->get_trust( ) >= ch->get_trust( ) ) ) {
        ch->send_to( "You failed.\n\r" );
        return;
    }
    
    if( newName.empty( ) ) {
        ch->send_to( "Rename to what new name?\n\r" );
        return;
    }

    if (oldName ^ newName) {
        if (!russianName.empty( )) {
            victim->setRussianName( russianName );
            ch->println( "Russian name set." );
        }
        else {
            ch->println( "Both names are equal!" );
        }

        return;
    }
    
    if (!badNames->checkName( newName )) {
        ch->send_to( "The new name is illegal.\n\r" );
        return;
    }
    
    if (PCharacterManager::find( newName )) {
        ch->send_to( "A player with that name already exists!\n\r" );
        return;                
    }

    // Переименовываем объекты
    Object *obj;
    Object *obj_next;
    for ( obj = object_list; obj != 0; obj = obj_next )
    {
            obj_next = obj->next;
            if (obj->hasOwner( victim ))
            {
                    obj->setOwner( newName.c_str( ) );
                    save_items_at_holder( obj );
            }
    }

    PCharacterManager::rename( victim->getName( ), newName );

    victim->setName( newName );
    victim->setRussianName( russianName );

    victim->save( );

    DLFile( dreamland->getPlayerDir( ), 
            oldName.toLower( ), 
            PCharacterManager::ext ).remove( );

    ch->send_to("Character renamed.\n\r");
    act_p("$c1 переименова$gло|л|ла тебя в $C4!",ch,0,victim,TO_VICT,POS_DEAD);
}

CMDWIZP( notitle )
{
  char arg[MAX_INPUT_LENGTH];
  Character *victim;

    if ( !ch->is_immortal() )
        return;
    argument = one_argument(argument,arg);

    if ( (victim = get_char_world(ch ,arg)) == 0 )
    {
        ch->send_to("He is not currently playing.\n\r");
        return;
    }

   if (IS_SET(victim->act, PLR_NO_TITLE) )
        {
   victim->act.removeBit(PLR_NO_TITLE);
   victim->send_to("You can change your title again.\n\r");
   ch->send_to("Ok.\n\r");
        }
   else
        {                
   victim->act.setBit(PLR_NO_TITLE);
   victim->send_to("You won't be able to change your title anymore.\n\r");
   ch->send_to("Ok.\n\r");
        }
   return;
}


CMDWIZP( noaffect )
{
    Affect *paf,*paf_next;
    char arg[MAX_INPUT_LENGTH];
    Character *victim;

    if ( !ch->is_immortal() )
        return;

    argument = one_argument(argument,arg);

    if ( (victim = get_char_world(ch ,arg)) == 0 )
    {
        ch->send_to("He is not currently playing.\n\r");
        return;
    }

    for ( paf = victim->affected; paf != 0; paf = paf_next )
    {
        paf_next        = paf->next;
        if ( paf->duration >= 0 )
        {
            if (paf->type->getAffect( ))
                paf->type->getAffect( )->remove( victim );

            affect_remove( victim, paf );
        }
    }
}

CMDWIZP( reboot )
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument(argument,arg);

 if (arg[0] == '\0')
    {
      ch->send_to("Usage: reboot now\n\r");
      ch->send_to("Usage: reboot <ticks to reboot>\n\r");
      ch->send_to("Usage: reboot cancel\n\r");
      ch->send_to("Usage: reboot status\n\r");
      return;
    }

    if (is_name(arg,"cancel"))
     {
      dreamland->setRebootCounter( -1 );
      ch->send_to("Reboot canceled.\n\r");
      return;
    }

    if (is_name(arg, "now"))
     {
      reboot_anatolia();
      return;
    }

    if (is_name(arg, "status"))
    {
      if (dreamland->getRebootCounter( ) == -1)
        sprintf(buf, "Automatic rebooting is inactive.\n\r");
      else
        sprintf(buf,"Reboot in %i minutes.\n\r", dreamland->getRebootCounter( ));
      ch->send_to(buf);
      return;
    }

    if (is_number(arg))
    {
     dreamland->setRebootCounter( atoi(arg) );
     sprintf(buf,"Dream Land will reboot in %i ticks.\n\r",dreamland->getRebootCounter( ));
     ch->send_to(buf);
     return;
    }

    run(ch, str_empty);
}


CMDWIZP( olevel )
{
    char buf[MAX_INPUT_LENGTH];
    char level[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found;
    int number = 0, max_found;

    found = false;
    number = 0;
    max_found = 200;

    argument = one_argument(argument, level);
    if (level[0] == '\0')
    {
        ch->send_to("Syntax: olevel <level>\n\r");
        ch->send_to("        olevel <level> <name>\n\r");
        return;
    }

    argument = one_argument(argument, name);
    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if ( obj->level != atoi(level) )
            continue;

        if ( name[0] != '\0' && !is_name(name, obj->getName( ) ) )
            continue;

        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj );

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by)
        &&   in_obj->carried_by->in_room != 0)
            sprintf( buf, "%3d) [%d] %s is carried by %s [Room %d]\n\r",
                number, obj->pIndexData->vnum, obj->getShortDescr('1').c_str( ),ch->sees(in_obj->carried_by, '5').c_str(),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != 0 && ch->can_see(in_obj->in_room))
            sprintf( buf, "%3d) [%d] %s is in %s [Room %d]\n\r",
                number, obj->pIndexData->vnum,obj->getShortDescr( '1' ).c_str( ),in_obj->in_room->name,
                in_obj->in_room->vnum);
        else
            sprintf( buf, "%3d) [%d]%s is somewhere\n\r",number, obj->pIndexData->vnum,obj->getShortDescr( '1' ).c_str( ));

        buf[0] = Char::upper(buf[0]);
        buffer << buf;

        if (number >= max_found)
            break;
    }

    if ( !found )
        ch->send_to("Nothing like that in heaven or earth.\n\r");
    else
        page_to_char(buffer.str( ).c_str( ), ch);
}

CMDWIZP( mlevel )
{
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Character *victim;
    bool found;
    int count = 0;

    if ( argument[0] == '\0' )
    {
        ch->send_to("Syntax: mlevel <level>\n\r");
        return;
    }

    found = false;

    for ( victim = char_list; victim != 0; victim = victim->next )
    {
        if ( victim->in_room != 0
        &&   atoi(argument) == victim->getRealLevel( ) )
        {
            found = true;
            count++;
            sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
                victim->is_npc() ? victim->getNPC()->pIndexData->vnum : 0,
                victim->getNameP( '1' ).c_str(),
                victim->in_room->vnum,
                victim->in_room->name );
            buffer << buf;
        }
    }

    if ( !found )
        act_p( "You didn't find any mob of level $T.", ch, 0, argument, TO_CHAR,POS_DEAD );
    else
        page_to_char(buffer.str( ).c_str( ), ch);
}

CMDWIZP( nopk )
{
  if( !*argument ) {
    if( dreamland->hasOption( DL_PK ) ) {
      ch->send_to("{RPK{x разрешен.\n\r");
    } else {
      ch->send_to("{RPK{x запрещен.\n\r");
    }
  } else {
    if( !str_cmp( argument, "off" ) ) {
      dreamland->setOption( DL_PK );
      ch->send_to("{RPK{x разрешен.\n\r");
    } else {
      if( !str_cmp( argument, "on" ) ) {
        dreamland->removeOption( DL_PK );
        ch->send_to("{RPK{x запрещен.\n\r");
      } else {
        ch->send_to("Синтаксис: nopk [on/off]");
        return;
      }
    }
  }
}

CMDWIZP( curse )
{
        char arg1[MAX_INPUT_LENGTH];
        Character *victim;
        char buf[MAX_STRING_LENGTH];
        int modif;

        if( !argument[0] )
        {
                ch->send_to("Используйте:\n\r   curse <игрок> <+/-количество> <причина>\n\r");
                return;
        }

        argument = one_argument( argument, arg1 );

        if ( !( victim = get_char_world(ch ,arg1 ) ) )
        {
                ch->send_to("Не присутствует в игре.\n\r");
                return;
        }
        if ( victim->is_npc() )
        {
                ch->send_to("Это не есть игрок.\n\r");
                return;
        }

        if ( !argument[0] )
        {
            ch->pecho( "%#^C1 хуже использует умения на %d%%.",
                       victim, 100 - victim->getPC( )->curse.getValue( ) );
            return;
        }

        argument = one_argument( argument, arg1 );

        if ( arg1[0] == '-' )
                modif = -1;
        else if ( arg1[0] == '+' )
                modif = 1;
        else
        {
                ch->send_to("Используйте:\n\r   curse <игрок> <+/-количество> <причина>\n\r");
                return;
        }

        modif *= atoi( arg1 + 1 );

        if ( !argument[0] )
        {
                ch->send_to("Используйте:\n\r   curse <игрок> <+/-количество> <причина>\n\r");
                return;
        }

        if ( victim->getPC( )->curse + modif < 0
                || victim->getPC( )->curse + modif > 100 )
        {
                ch->send_to("Проклятие должно лежать в пределах 0..100\n\r");
                return;
        }

        if ( victim->getPC( )->bless )
        {
                ch->send_to("Ты не можешь проклясть благословленного.\n\r");
                return;
        }

        victim->getPC( )->curse += modif;
        interpret_raw( ch, "noaffect", victim->getNameP() );
        if ( modif > 0 )
                sprintf( buf, "Боги становятся более блакосклонны к тебе.\n\rПричина: %s\n\r",
                        argument );
        else
                sprintf( buf, "Боги проклинают тебя.\n\rПричина: %s", argument );
                        victim->send_to(buf);
}

CMDWIZP( bless )
{
        char arg1[MAX_INPUT_LENGTH];
        Character *victim;
        char buf[MAX_STRING_LENGTH];
        int modif;

        if( !argument[0] )
        {
                ch->send_to("Используйте:\n\r   bless <игрок> <+/-количество> <причина>\n\r");
                return;
        }

        argument = one_argument( argument, arg1 );

        if ( !( victim = get_char_world(ch ,arg1 ) ) )
        {
                ch->send_to("Не присутствует в игре.\n\r");
                return;
        }
        if ( victim->is_npc() )
        {
                ch->send_to("Это не есть игрок.\n\r");
                return;
        }

        if ( !argument[0] )
        {
            ch->pecho( "%#^C1 лучше использует умения на %d%%.",
                       victim, 100 - victim->getPC( )->bless.getValue( ) );
            return;
        }

        argument = one_argument( argument, arg1 );

        if ( arg1[0] == '-' )
                modif = -1;
        else if ( arg1[0] == '+' )
                modif = 1;
        else
        {
                ch->send_to("Используйте:\n\r   bless <игрок> <+/-количество> <причина>\n\r");
                return;
        }

        modif *= atoi( arg1 + 1 );

        if ( !argument[0] )
        {
                ch->send_to("Используйте:\n\r   bless <игрок> <+/-количество> <причина>\n\r");
                return;
        }

        if ( victim->getPC( )->bless + modif < 0
                || victim->getPC( )->bless + modif > 50 )
        {
                ch->send_to("Благословение должно лежать в пределах 0..50\n\r");
                return;
        }

        if ( victim->getPC( )->curse != 100 )
        {
                ch->send_to("Ты не можешь благословить проклятого.\n\r");
                return;
        }

        victim->getPC( )->bless += modif;
        interpret_raw( ch, "noaffect", victim->getNameP( ) );
        if ( modif > 0 )
                sprintf( buf, "Ты чувствуешь {CБожественное Благословение{x!\n\rПричина: %s\n\r",
                        argument );
        else
                sprintf( buf, "Ты теряешь расположение Богов.\n\rПричина: %s", argument );
                        victim->send_to(buf);
}


CMDWIZP( merchant )
{
        char buf[MAX_STRING_LENGTH];
        sprintf(buf,"Состояние всемирного банка : {Y%ld gold{x\n\r",
                dreamland->getBalanceMerchantBank());
        ch->send_to(buf);
}


extern int nHitString, sHitString, nDups, sDups;

extern int memAllocCount, memAllocSize;

CMDWIZP( memory )
{
    char buf[MAX_STRING_LENGTH];

    sprintf( buf, "Affects %5d\n\r", top_affect    ); ch->send_to(buf);
    sprintf( buf, "Areas   %5d\n\r", top_area      ); ch->send_to(buf);
    sprintf( buf, "ExDes   %5d\n\r", top_ed        ); ch->send_to(buf);
    sprintf( buf, "Exits   %5d\n\r", top_exit      ); ch->send_to(buf);
    sprintf( buf, "Helps   %5d\n\r", helpManager->getArticles( ).size( ) ); ch->send_to(buf);
//    sprintf( buf, "Socials %5d\n\r", SocialManager::getThis( )->getSocialCount( ) ); ch->send_to(buf);
    sprintf( buf, "Mobs    %5d(%d new format)\n\r", top_mob_index,newmobs );
    ch->send_to(buf);
    sprintf( buf, "(in use)%5d\n\r", mobile_count  ); ch->send_to(buf);
    sprintf( buf, "Objs    %5d(%d new format)\n\r", top_obj_index,newobjs );
    ch->send_to(buf);
    sprintf( buf, "Resets  %5d\n\r", top_reset     ); ch->send_to(buf);
    sprintf( buf, "Rooms   %5d\n\r", top_room      ); ch->send_to(buf);

    sprintf( buf, "Allocs  %5d calls   of %7d bytes.\n\r",
        memAllocCount, memAllocSize);
    ch->send_to(buf);
    sprintf( buf, "Perms   %5d blocks  of %7d bytes.\n\r",
        nAllocPerm, sAllocPerm );
    ch->send_to(buf);
}

CMDWIZP( dump )
{
    int count,count2,num_pcs,aff_count;
    Character *fch;
    MOB_INDEX_DATA *pMobIndex;
    Object *obj;
    OBJ_INDEX_DATA *pObjIndex;
    Room *room;
    EXIT_DATA *exit;
    Descriptor *d;
    Affect *af;
    FILE *fp;
    int vnum,nMatch = 0;

    /* open file */
    fp = fopen("mem.dmp","w");

    /* report use of data structures */

    num_pcs = 0;
    aff_count = 0;

    /* mobile prototypes */
    fprintf(fp,"MobProt        %4d (%8d bytes)\n",
        top_mob_index, top_mob_index * (sizeof(*pMobIndex)));

    /* mobs */
    count = 0;  count2 = 0;
    for (fch = char_list; fch != 0; fch = fch->next)
    {
        count++;
        if (fch->getPC( ) != 0)
            num_pcs++;
        for (af = fch->affected; af != 0; af = af->next)
            aff_count++;
    }

    fprintf(fp,"Mobs        %4d (%8d bytes), %2d free (%d bytes)\n",
        count, count * (sizeof(*fch)), count2, count2 * (sizeof(*fch)));

    /* descriptors */
    count = 0; count2 = 0;
    for (d = descriptor_list; d != 0; d = d->next)
        count++;

    fprintf(fp, "Descs        %4d (%8d bytes), %2d free (%d bytes)\n",
        count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

    /* object prototypes */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
        if ( ( pObjIndex = get_obj_index( vnum ) ) != 0 )
        {
            for (af = pObjIndex->affected; af != 0; af = af->next)
                aff_count++;
            nMatch++;
        }

    fprintf(fp,"ObjProt        %4d (%8d bytes)\n",
        top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


    /* objects */
    count = 0;  count2 = 0;
    for (obj = object_list; obj != 0; obj = obj->next)
    {
        count++;
        for (af = obj->affected; af != 0; af = af->next)
            aff_count++;
    }

    /* rooms */
    fprintf(fp,"Rooms        %4d (%8d bytes)\n",
        top_room, top_room * (sizeof(*room)));

     /* exits */
    fprintf(fp,"Exits        %4d (%8d bytes)\n",
        top_exit, top_exit * (sizeof(*exit)));

    fclose(fp);

    /* start printing out mobile data */
    fp = fopen("mob.dmp","w");

    fprintf(fp,"\nMobile Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_mob_index; vnum++)
        if ((pMobIndex = get_mob_index(vnum)) != 0)
        {
            nMatch++;
            fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
                pMobIndex->vnum,pMobIndex->count,
                pMobIndex->killed,
                russian_case( pMobIndex->short_descr, '1' ).c_str( ));
        }
    fclose(fp);

    /* start printing out object data */
    fp = fopen("obj.dmp","w");

    fprintf(fp,"\nObject Analysis\n");
    fprintf(fp,  "---------------\n");
    nMatch = 0;
    for (vnum = 0; nMatch < top_obj_index; vnum++)
        if ((pObjIndex = get_obj_index(vnum)) != 0)
        {
            nMatch++;
            fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
                pObjIndex->vnum,pObjIndex->count,
                pObjIndex->reset_num,
                russian_case( pObjIndex->short_descr, '1' ).c_str( ));
        }

    /* close file */
    fclose(fp);
    ch->println( "MUD statistics dumped." );
}


CMDWIZP( version )
{
    ch->send_to( dreamland->getVersion( ) );
}

/*------------------------------------------------------------------
 *
 *-----------------------------------------------------------------*/

extern "C"
{
    SO::PluginList initialize_cmd_wizard( )
    {
        SO::PluginList ppl;
        return ppl;
    }
}


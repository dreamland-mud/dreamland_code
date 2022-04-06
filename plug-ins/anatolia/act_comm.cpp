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
 *    Zadvinsky Aleksandr  {Kiddy}                                         *
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
 **************************************************************************/
/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#include "dreamland.h"
#include "pcharactermanager.h"
#include "logstream.h"
#include "dlfileop.h"
#include "integer.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "behavior_utils.h"
#include "commandtemplate.h"
#include "commonattributes.h"
#include "affecthandler.h"
#include "skill.h"

#include "objectbehavior.h"
#include "wiznet.h"
#include "infonet.h"
#include "messengers.h"
#include "descriptorstatemanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "race.h"
#include "room.h"
#include "save.h"
#include "merc.h"
#include "descriptor.h"
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "object.h"
#include "character.h"
#include "clanreference.h"
#include "gsn_plugin.h"

#include "vnum.h"
#include "act_move.h"
#include "handler.h"
#include "def.h"

CLAN(invader);
CLAN(none);
bool password_check( PCMemoryInterface *pci, const DLString &plainText );


void delete_player( PCharacter *victim ) 
{
    DLString name = victim->getName( );

    victim->getAttributes( ).getAttr<XMLStringAttribute>( "quit_flags" )->setValue( "quiet count forced" );
    do_quit( victim, "" );
    PCharacterManager::pfDelete( name );
}

/* RT code to delete yourself */

CMDRUNP( delete )
{
    PCharacter *pch;
    
    if ( ch->is_npc() )
           return;
    
    pch = ch->getPC( );

    if (pch->confirm_delete)
    {
        if (!password_check( pch, argument ))
        {
            pch->pecho("Попытка суицида отменена -- неверный пароль.");
            pch->confirm_delete = false;
            return;
        }
        else
        {
            wiznet( WIZ_SECURE, 0, pch->get_trust( ), 
                   "%1$^C1 превращает себя в помехи в проводах.", pch );
			DLString msg;
			msg = fmt(0, "{1{C%1$^C1 идет по пути Арханта и совершает суицид, навсегда покидая этот мир.", pch);
        	infonet(pch, 0, "{CТихий голос из $o2: ", msg.c_str());
        	send_to_discord_stream(":ghost: " + msg); // discord only here, explicitly asked for by players		
            delete_player( pch );
            return;
        }
    }

    pch->pecho("{RВНИМАНИЕ: {WЭТО НЕОБРАТИМОЕ ДЕЙСТВИЕ, ТВОЙ ПЕРСОНАЖ БУДЕТ УДАЛЕН НАСОВСЕМ!{x");	
    pch->pecho("Введи {y{ledelete{lrудалить{lx <твой пароль>{x для подтверждения команды.");
    pch->pecho("Чтобы отменить попытку суицида, введи {y{ledelete{lrудалить{lx без пароля.");
    pch->getPC( )->confirm_delete = true;
    wiznet( WIZ_SECURE, 0, pch->get_trust( ), 
            "%^C1 собирается удалить своего персонажа.", pch );
}

/* COMPAT */void do_quit( Character *ch, const char *argument )
{
    interpret_raw( ch, "quit", argument );
}

/* COMPAT */void do_yell( Character *ch, const char *argument )
{
    interpret_raw( ch, "yell", argument );
}

/* COMPAT */void do_say( Character *ch, const char *argument )
{
    interpret_raw( ch, "say", argument );
}



CMDRUNP( rent )
{
    ch->pecho("Здесь нет ренты. Просто покинь мир.");
    return;
}

static bool oprog_quit( Object *obj, Character *ch, bool count )
{
    Object *o, *o_next;
    
    FENIA_CALL( obj, "Quit", "Ci", ch, count );
    FENIA_NDX_CALL( obj, "Quit", "OCi", obj, ch, count );
    BEHAVIOR_CALL( obj, quit, ch, count );

    for (o = obj->contains; o; o = o_next) {
        o_next = o->next_content;
        oprog_quit( o, ch, count );
    }

    return false;
}

CMDRUNP( quit )
{
    PCharacter *pch;
    Object *obj, *obj_next;
    bool fCount, fQuiet, fForced, fAuto;

    pch = ch->getPC( );

    if (!pch)
        return;

    if (pch->desc && pch->desc->connected == CON_NANNY) {
        pch->desc->close( );
        return;
    }
   
    /*
     * Parse various quit flags passed as a player attribute, for situations
     * when quit is caused by deny, delete etc.
     */
    fCount = fQuiet = fForced = fAuto = false;
    XMLStringAttribute::Pointer quitFlagsAttr = pch->getAttributes( ).findAttr<XMLStringAttribute>( "quit_flags" );
    if (quitFlagsAttr) {
        pch->getAttributes( ).eraseAttribute( "quit_flags" );
        pch->save( );

        DLString arg, args = quitFlagsAttr->getValue( );
        while (!( arg = args.getOneArgument( ) ).empty( )) {
            if (arg == "count")
                fCount = true;
            else if (arg == "quiet")
                fQuiet = true;
            else if (arg == "forced")
                fForced = true;
            else if (arg == "auto")
                fAuto = true;
        }
    }
    
    if (pch->switchedTo)
        interpret_raw( pch->switchedTo, "return" );
    
    if (pch->position == POS_FIGHTING || pch->fighting) {
        if (!fForced) {
            pch->pecho( "Не сейчас! Тебе необходимо закончить сражение!");
            return;
        }

        stop_fighting( pch, true );
    }

    if (pch->position  < POS_STUNNED) {
        if (!fForced) {
            pch->pecho("Ты еще не УМЕ%GРЛО|Р|РЛА.", pch);
            return;
        }

        pch->position = POS_STANDING;
        pch->hit = std::max( 1, (int)pch->hit );
    }
    
    if (!pch->is_immortal( ) && !fAuto && !fForced) {
        if (IS_VIOLENT(pch)) {
            pch->pecho("У тебя слишком много адреналина в крови.");
            return;
        }
        if (IS_SLAIN(pch)) {
            pch->pecho("Правда о твоем поражении еще не забыта.");
            return;
        }
        if (IS_KILLER(pch)) {
            pch->pecho("Боги еще помнят убийство, совершенное тобой.");
            return;
        }
    }

    if (IS_CHARMED(pch)) {
        if (!fForced) {
            pch->pecho( "Ты не можешь покинуть сво%1$Gего|его|ю хозя%1$Gина|ина|йку.", pch->master);
            return;
        }

        pch->stop_follower( );
    }

    if (!fForced 
        && !pch->is_immortal( )
        && IS_SET( pch->act, PLR_NO_EXP ))
    {
        pch->pecho("Ты не можешь покинуть этот мир! Твой дух во власти противника.");
        return;
    }

    if (auction->item != 0 && ((pch == auction->buyer) || (pch == auction->seller))) {
        if (!fForced) {
            pch->pecho("Подожди пока вещь, выставленная на аукцион, будет продана или возвращена.");
            return;
        }
        
        obj_to_char( auction->item, auction->seller );
        auction->item = 0;
    }

    if (!fForced
        && !pch->is_immortal()
        && IS_ROOM_AFFECTED( pch->in_room, AFF_ROOM_ESPIRIT ))
    {
        pch->pecho("Злые духи в этой зоне не отпускают тебя.");
        return;
    }

    if (!fForced
            && !pch->is_immortal() 
            && pch->getClan( ) != clan_invader 
            && pch->isAffected( gsn_evil_spirit ))
    {
        pch->pecho("Злые духи, овладевшие тобой, не позволяют тебе покинуть этот мир.");
        return;
    }

    if (!fForced
        && !pch->is_immortal()  
        && pch->isAffected(gsn_suspect))
    {
        pch->pecho("Ты не можешь этого сделать -- тебя ждет Суд!");
        return;
    }

    if (!fForced
            && !pch->is_immortal( ) 
            && pch->death_ground_delay > 0
            && pch->trap.isSet( TF_NO_MOVE ))
    {
        pch->pecho("Сначала выберись из ловушки, а потом можно и покинуть этот мир.");
        return;
    }

    if (!pch->is_immortal( )  
            && pch->in_room->pIndexData->clan != clan_none 
            && pch->getClan() != pch->in_room->pIndexData->clan)
    {
        if (!fAuto && !fForced) {
            pch->pecho("Ты не можешь этого сделать -- здесь не твоя территория!");
            return;
        }
        
        transfer_char( pch, 0, get_room_instance( ROOM_VNUM_TEMPLE ) );
        if (pch->pet)
            transfer_char( pch->pet, 0, pch->in_room );
    }

    undig( pch );
    pch->dismount( );

    interpret_raw( pch, "save", "" );
    
    PCharacterManager::quit( pch );

    pch->pecho("Жаль, но все хорошее когда-нибудь заканчивается.");
    
    if (pch->desc && !fQuiet)
        DescriptorStateManager::getThis( )->handle( CON_PLAYING, CON_QUIT, pch->desc );

    oldact_p("$c1 покину$gло|л|ла этот мир.", pch, 0, 0, TO_ROOM ,POS_DEAD);

    if (!pch->getPC( )->getAttributes( ).isAvailable("quietLogin")) {
        wiznet( WIZ_LOGINS, 0, pch->get_trust( ), "%1$^C1 покину%1$Gло|л|ла этот мир.", pch );
        infonet(pch, 0, "{CТихий голос из $o2: ", "{W%1$C1 покину%1$Gло|л|ла Dream Land.{x", pch);
        send_discord_orb(fmt(0, "%1$^C1 покинул%1$Gо||а Dream Land.", pch));
    }
	
    dreamland->removeOption( DL_SAVE_OBJS );

    for( obj = pch->carrying; obj ; obj = obj_next)
    {
        obj_next = obj->next_content;

        if (oprog_quit( obj, pch, fCount ))
            continue;
    }

    dreamland->resetOption( DL_SAVE_OBJS );

    save_items( pch->in_room );

    /*
     * After extract_char the pch is no longer valid!
     */
    pch->save();
    
    Descriptor *dtmp = pch->desc;
    extract_char( pch, fCount );

    if (dtmp)
        dtmp->close( );
}



CMDRUNP( save )
{
    if( ch->is_npc() ) 
        return;

    ch->getPC( )->save();
    ch->pecho("Архивариус {CМира Мечты{x заносит сведения о тебе в свои манускрипты.");
    ch->setWaitViolence( 1 );
}


/* $Id$
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "globalchannel.h"
#include "messengers.h"

#include "skillreference.h"
#include "npcharacter.h"
#include "pcharacter.h"

#include "dreamland.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "descriptor.h"
#include "loadsave.h"
#include "ban.h"
#include "act.h"
#include "merc.h"
#include "def.h"

GSN(deafen);
LANG(common);

bool has_nochannel(Character *ch)
{
    static const DLString nochannel( "nochannel" );
    
    if (ch->is_npc( ))
        return false;

    if (ch->getPC( )->getAttributes( ).isAvailable( nochannel ))
        return true;
    
    if (ch->desc && banManager->check( ch->desc, BAN_COMMUNICATE )) 
        return true;

    return false;
}   

/*-----------------------------------------------------------------------
 * GlobalChannel
 *-----------------------------------------------------------------------*/
GlobalChannel::GlobalChannel( )
           : soap( false ), translate( false ), nochannel( false ), 
             deafenOther( false ), quiet( false ), 
             nomob( false ), confirmed( false ), dig( false )
{
}

void GlobalChannel::run( Character *ch, const DLString &arg )
{
    Listeners listeners;
    
    if (!msgDisable.empty( )) {
        ch->pecho( msgDisable );
        return;
    }
    
    if (arg.empty( ) && msgOtherNoarg.empty( )) {
        if (off == 0 && !msgSelfNoarg.empty( )) {
            ch->pecho( msgSelfNoarg );
            return;
        }
        
        TOGGLE_BIT(ch->comm, off);

        if (!IS_SET(ch->comm, off)) {
            if (msgOn.empty( ))
                ch->printf( "Канал %s теперь включен.\r\n", getName( ).c_str( ) );
            else
                ch->pecho( msgOn );
        }
        else {
            if (msgOff.empty( ))
                ch->printf( "Канал %s теперь выключен.\r\n", getName( ).c_str( ) );
            else
                ch->pecho( msgOff );
        }

        return;
    }
    
    if (!canTalkGlobally( ch ))
        return;

    REMOVE_BIT(ch->comm, off);

    findListeners( ch, listeners );
    
    if (needOutputSelf( ch )) {
        bool fMild = (IS_SET(ch->comm, COMM_MILDCOLOR) && !msgSelfMild.empty( ));
        bool fEmpty = (!msgListEmpty.empty( ) && listeners.size( ) == 0);
        const DLString &fmtSelf = fEmpty ? msgListEmpty : fMild ? msgSelfMild : msgSelf;

        DLString outSelf = arg;
        applyGarble( ch, outSelf, ch );

        DLString message = outputSelf( ch, fmtSelf, outSelf );
        ch->pecho(message);
        postOutput(ch, message);
    }
    
    if (needOutputOther( ch )) {
        Listeners::iterator i;

        for (i = listeners.begin( ); i != listeners.end( ); i++) {
            Character *victim = *i;
            bool fNoarg = (arg.empty( ) && !msgOtherNoarg.empty( ));
            bool fMild = (IS_SET((*i)->comm, COMM_MILDCOLOR) && !msgOtherMild.empty( ));
            const DLString &fmtVict = fNoarg ? msgOtherNoarg : fMild ? msgOtherMild : msgOther;
            
            DLString outVict = arg;
            applyGarble( ch, outVict, victim );
            applyTranslation( ch, outVict, victim );

            DLString message = outputVict( ch, victim, fmtVict, outVict );
            victim->pecho(message);
            postOutput(victim, message);
        }
    }

    triggers( ch, arg );
}

bool GlobalChannel::canTalkGlobally( Character *ch ) const
{
    if (nomob && ch->is_npc( )) {
        ch->pecho( "Этот канал не для тебя, прости." );
        return false;
    }
    
    if (ch->get_trust( ) < trustSpeak) {
        ch->pecho( "В этом канале ты можешь только слушать, прости." );
        return false;
    }
    
    if (quiet && IS_SET(ch->comm, COMM_QUIET)) {
        ch->pecho( "Сначала необходимо повынимать вату из ушей.");
        return false;
    }

    if (checkNoChannel( ch ))
        return false;

    if (checkConfirmed( ch ))
        return false;

    if (checkSoap( ch ))
        return false;

    if (!ch->is_immortal( )) {
        int cost = ch->max_mana * manaPercent / 100;

        if (ch->mana < cost) {
            ch->pecho( "У тебя недостаточно сил, чтобы орать на весь мир." );
            return false;
        }

        ch->mana -= cost;
    }

    return true;
}

bool GlobalChannel::isGlobalListener( Character *ch, Character *victim ) const
{
    if (victim == ch)
        return false;
    
    if (!canHear( victim ))
        return false;
    
    if (!victim->can_sense( ch ))
        return false;

    if (IS_SET(victim->comm, off))
        return false;
    
    if (quiet && IS_SET(victim->comm, COMM_QUIET))
        return false;
    
    if (victim->position < positionOther)
        return false;
    
    if (deafenOther && victim->isAffected(gsn_deafen ))
        return false;

    if (checkIgnore( ch, victim ))
        return false;
    
    if (checkIsolator( ch, victim ))
        return false;

    return true;
}

bool GlobalChannel::checkConfirmed( Character *ch ) const
{
    if (!confirmed)
        return false;

    if (IS_CHARMED(ch))
        return checkConfirmed( ch->master );

    if (!ch->is_npc( ) && !IS_SET(ch->act, PLR_CONFIRMED))
    {
        ch->pecho("Только подтвержденные богами персонажи могут общаться в этом канале." );
        return true;
    }

    return false;
}

bool GlobalChannel::checkNoChannel( Character *ch ) const
{
    if (!nochannel)
        return false;
    
    if (has_nochannel( ch )) {
        if (!msgNochan.empty( ))
            ch->pecho( msgNochan );
        else
            ch->pecho( "Боги лишили тебя возможности общаться." );
        
        return true;
    }

    if (IS_CHARMED(ch) && has_nochannel( ch->master )) {
        act("%^C1 сдавленно хрипит, не в силах вымолвить ни слова.", ch, 0, 0, TO_ROOM);        
        return true;
    }

    return false;
}

bool GlobalChannel::checkSoap( Character *ch ) const
{
    static const DLString attrName( "soap" );
    
    if (!soap)
        return false;

    if (IS_CHARMED(ch))
        return checkSoap( ch->master );
    
    if (ch->is_npc( ))
        return false;
    
    if (!ch->getPC( )->getAttributes( ).isAvailable( attrName )) 
        return false;
    
    act("%^C1 пускает изо рта {Rр{Yа{Gз{Cн{Mо{Rц{Gв{Yе{Cт{Mн{Yы{Cе{x мыльные пузыри.", ch, 0, 0, TO_ROOM);
    act("Ты пускаешь изо рта {Rр{Yа{Gз{Cн{Mо{Rц{Gв{Yе{Cт{Mн{Yы{Cе{x мыльные пузыри.", ch, 0, 0, TO_CHAR);
    return true;
}

void GlobalChannel::applyTranslation( Character *ch, DLString &msg, Character *victim ) const
{
    if (!translate)
        return;

    msg = ch->language->translate( msg, ch, victim );

    if (ch->language != lang_common)
        msg = DLString( "[" ) + ch->language->getName( ) + "] " + msg;
}

void GlobalChannel::triggers( Character *ch, const DLString &msg ) const
{
    if (dreamland->hasOption( DL_LOG_COMM ) && getLog( ) != LOG_NEVER)
        LogStream::sendNotice( ) << "channel [" << getName( ) << "] " << ch->getName( ) << ": " << msg << endl;

    if (!msg.empty() && hook) {
        if (ooc)
            send_discord_ooc(ch, msgOther, msg);
        else
            send_discord_ic(ch, msgOther, msg);
    }
}

bool GlobalChannel::needOutputSelf( Character *ch ) const
{
    if (deafen && ch->isAffected( gsn_deafen ))
        return false;
    
    if (ch->desc && !ch->desc->echo)
        return false;

    return true;
}

bool GlobalChannel::needOutputOther( Character *ch ) const
{
    if (dig && DIGGED(ch)) {
        ch->pecho( "Стены могилы поглощают звуки." );
        return false;
    }
    
    if (ch->desc && !ch->desc->echo)
        return false;

    return true;
}


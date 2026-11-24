/* $Id$
 *
 * ruffina, 2004
 */
#include <jsoncpp/json/json.h>
#include <map>
#include <vector>

#include "logstream.h"
#include "globalchannel.h"
#include "chatframe.h"
#include "messengers.h"
#include "configurable.h"
#include "fenia_utils.h"
#include "lang.h"

#include "skillreference.h"
#include "npcharacter.h"
#include "pcharacter.h"

#include "dreamland.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "descriptor.h"
#include "loadsave.h"
#include "accountmanager.h"
#include "ban.h"
#include "act.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

GSN(deafen);

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
             nomob( false ), confirmed( false ), dig( false ), grumble( false )
{
}

/*-----------------------------------------------------------------------
 * Old-man noises
 *-----------------------------------------------------------------------*/
// config/aging.json "grumble": odds by the speaker's true hours over all lives
// ("tiers": [{hours, percent}], the highest reached tier wins) and one word list
// per language. The lists are parallel: one index is the same joke for every
// reader, whatever language each of them reads in. A drunk speaker grumbles
// "drunkFactor" times as often, up to "drunkCap" percent, and the noise is
// slurred like the line it trails.
//
// The file is hand-edited, and jsoncpp throws std::exception on a wrong type,
// which nothing on the command path catches. So it is validated once here into
// plain containers and the say path never touches a Json::Value.
static vector<pair<int, int> > grumbleTiers;
static map<DLString, vector<DLString> > grumbleWords;
static const int GRUMBLE_DRUNK_FACTOR = 2;
static const int GRUMBLE_DRUNK_CAP = 25;
static int grumbleDrunkFactor = GRUMBLE_DRUNK_FACTOR;
static int grumbleDrunkCap = GRUMBLE_DRUNK_CAP;

// An integer field of the grumble section, or the default when missing or invalid.
static int grumble_int( const Json::Value &cfg, const char *key, int def, int lo, int hi )
{
    if (!cfg.isMember( key ))
        return def;

    const Json::Value &v = cfg[key];
    if (!v.isInt( ) || v.asInt( ) < lo || v.asInt( ) > hi) {
        LogStream::sendError( ) << "aging.json: grumble." << key << " needs an integer "
                                << lo << ".." << hi << ", using " << def << endl;
        return def;
    }

    return v.asInt( );
}

CONFIGURABLE_LOADED(config, aging)
{
    grumbleTiers.clear( );
    grumbleWords.clear( );
    grumbleDrunkFactor = GRUMBLE_DRUNK_FACTOR;
    grumbleDrunkCap = GRUMBLE_DRUNK_CAP;

    if (!value.isObject( )) {
        if (!value.isNull( ))
            LogStream::sendError( ) << "aging.json: root is not an object, no grumble" << endl;
        return;
    }

    if (!value.isMember( "grumble" ))
        return;

    const Json::Value &cfg = value["grumble"];
    if (!cfg.isObject( )) {
        LogStream::sendError( ) << "aging.json: grumble is not an object, no grumble" << endl;
        return;
    }

    grumbleDrunkFactor = grumble_int( cfg, "drunkFactor", GRUMBLE_DRUNK_FACTOR, 1, 10 );
    grumbleDrunkCap = grumble_int( cfg, "drunkCap", GRUMBLE_DRUNK_CAP, 0, 100 );

    for (Json::Value::const_iterator k = cfg.begin( ); k != cfg.end( ); k++) {
        DLString key = k.name( );
        const Json::Value &list = *k;

        if (key == "drunkFactor" || key == "drunkCap")
            continue;

        if (!list.isArray( )) {
            LogStream::sendError( ) << "aging.json: grumble." << key << " is not a list, skipped" << endl;
            continue;
        }

        for (Json::ArrayIndex i = 0; i < list.size( ); i++) {
            const Json::Value &item = list[i];

            if (key == "tiers") {
                if (item.isObject( )
                    && item["hours"].isInt( ) && item["percent"].isInt( ))
                    grumbleTiers.push_back( make_pair( item["hours"].asInt( ), item["percent"].asInt( ) ) );
                else
                    LogStream::sendError( ) << "aging.json: grumble.tiers[" << i
                                            << "] needs integer hours and percent, skipped" << endl;
            }
            else if (item.isString( ))
                grumbleWords[key].push_back( item.asString( ) );
            else
                LogStream::sendError( ) << "aging.json: grumble." << key << "[" << i
                                        << "] is not a string, skipped" << endl;
        }
    }
}

static const vector<DLString> * grumble_words( const DLString &lang )
{
    map<DLString, vector<DLString> >::const_iterator w = grumbleWords.find( lang );
    if (w == grumbleWords.end( ) || w->second.empty( ))
        return 0;
    return &w->second;
}

int GlobalChannel::grumbleRoll( Character *ch ) const
{
    if (!grumble || ch->is_npc( ))
        return -1;

    if (IS_SET(ch->getPC( )->config, CONFIG_NOGRUMBLE))
        return -1;

    // Gibberish gets no clean tail.
    if (garblesMagic( ch ))
        return -1;

    const vector<DLString> *words = grumble_words( "ru" );
    if (!words || grumbleTiers.empty( ))
        return -1;

    int hours = pc_total_hours( ch->getPC( ) );
    int pct = 0;
    for (const pair<int, int> &tier: grumbleTiers)
        if (hours >= tier.first)
            pct = max( pct, tier.second );

    // Drunk old men grumble more, never less than sober.
    if (slurLevel( ch ) > 0)
        pct = max( pct, min( pct * grumbleDrunkFactor, grumbleDrunkCap ) );

    if (pct <= 0 || number_percent( ) > pct)
        return -1;

    return number_range( 0, (int)words->size( ) - 1 );
}

DLString GlobalChannel::grumbleText( int index, Character *reader, int slur ) const
{
    if (index < 0)
        return DLString::emptyString;

    const vector<DLString> *words = grumble_words( lang2attr( viewerLang( reader ) ) );
    if (!words)
        words = grumble_words( "ru" );
    if (!words)
        return DLString::emptyString;

    const DLString &word = (*words)[index % words->size( )];
    return DLString( " ... " ) + (slur > 0 ? slur_text( word, slur ) : word);
}

/*
 * The channel's off bit can also be a config option the account owns
 * (comm 'nomusic' is option 'nomusic'): the account then has to hear about the
 * change, or the next login puts the old value back.
 */
void GlobalChannel::writeOffThrough( Character *ch ) const
{
    if (ch->is_npc( ))
        return;

    DLString key = off.names( );
    if (!AccountManager::isAccountWideConfig( key ))
        return;

    AccountManager::writeConfigThrough( ch->getPC( ), key,
                                        Json::Value( IS_SET(ch->comm, off) != 0 ), true );
}

void GlobalChannel::run( Character *ch, const DLString &arg )
{
    Listeners listeners;
    
    if (!msgDisable.empty( )) {
        ch->pecho( localized( msgDisable, ch ) );
        return;
    }

    if (arg.empty( ) && msgOtherNoarg.empty( )) {
        if (off == 0 && !msgSelfNoarg.empty( )) {
            ch->pecho( localized( msgSelfNoarg, ch ) );
            return;
        }

        TOGGLE_BIT(ch->comm, off);
        writeOffThrough( ch );

        if (!IS_SET(ch->comm, off)) {
            if (msgOn.empty( ))
                ch->pecho( _("Канал %s теперь включен."), getName( ).c_str( ) );
            else
                ch->pecho( localized( msgOn, ch ) );
        }
        else {
            if (msgOff.empty( ))
                ch->pecho( _("Канал %s теперь выключен."), getName( ).c_str( ) );
            else
                ch->pecho( localized( msgOff, ch ) );
        }

        return;
    }
    
    if (!canTalkGlobally( ch ))
        return;

    if (IS_SET(ch->comm, off)) {
        REMOVE_BIT(ch->comm, off);
        writeOffThrough( ch );
    }

    findListeners( ch, listeners );

    DLString argGarbled = arg;
    applyGarble(ch, argGarbled);
    stripWeb(ch, argGarbled);

    // Readers get the noise, triggers and logs get the clean line.
    int noise = arg.empty( ) ? -1 : grumbleRoll( ch );
    int noiseSlur = slurLevel( ch );
    bool noiseRead = false;

    if (needOutputSelf( ch )) {
        bool fMild = (IS_SET(ch->comm, COMM_MILDCOLOR) && !msgSelfMild.empty( ));
        bool fEmpty = (!msgListEmpty.empty( ) && listeners.size( ) == 0);
        const DLString &fmtSelf = fEmpty ? msgListEmpty : fMild ? msgSelfMild : msgSelf;

        DLString noiseSelf = grumbleText( noise, ch, noiseSlur );
        DLString outSelf = argGarbled + noiseSelf;
        noiseRead = noiseRead || !noiseSelf.empty( );
        DLString message = outputSelf( ch, fmtSelf, outSelf );
        ch->pecho(message);
        postOutput(ch, message, ch);
        // The speaker's own copy: no counterpart on an undirected channel.
        //
        // Asked first, because the arguments are not free: webArea() builds the
        // area name in the reader's language, and a world with no panels open
        // at all should pay nothing for this line.
        if (chat_subscribed( ch ))
            chat_emit(ch, 0, true, getName( ), webKind( ), message, webArea( ch, ch ));
    }
    
    if (needOutputOther( ch )) {
        Listeners::iterator i;

        for (i = listeners.begin( ); i != listeners.end( ); i++) {
            Character *victim = *i;
            bool fNoarg = (arg.empty( ) && !msgOtherNoarg.empty( ));
            bool fMild = (IS_SET((*i)->comm, COMM_MILDCOLOR) && !msgOtherMild.empty( ));
            const DLString &fmtVict = fNoarg ? msgOtherNoarg : fMild ? msgOtherMild : msgOther;
            
            DLString noiseVict = grumbleText( noise, victim, noiseSlur );
            DLString outVict = argGarbled + noiseVict;
            noiseRead = noiseRead || !noiseVict.empty( );

            DLString message = outputVict( ch, victim, fmtVict, outVict );
            victim->pecho(message);
            postOutput(victim, message, ch);
            // Everything that could silence this listener -- the channel's own
            // switch, quiet, the deafen spell, trust, position, the isolator --
            // was decided in isGlobalListener before they got here, so a frame
            // goes exactly where the line went.
            if (chat_subscribed( victim ))
                chat_emit(victim, ch, false, getName( ), webKind( ), message, webArea( victim, ch ));
        }
    }

    if (noiseRead)
        achievement( ch, "back_in_my_day" );

    triggers( ch, argGarbled );
}

bool GlobalChannel::canTalkGlobally( Character *ch ) const
{
    if (nomob && ch->is_npc( )) {
        ch->pecho( _("Этот канал не для тебя, прости.") );
        return false;
    }
    
    if (ch->get_trust( ) < trustSpeak) {
        ch->pecho( _("В этом канале ты можешь только слушать, прости.") );
        return false;
    }
    
    if (quiet && IS_SET(ch->comm, COMM_QUIET)) {
        ch->pecho( _("Сначала необходимо повынимать вату из ушей."));
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
            ch->pecho( _("У тебя недостаточно сил, чтобы орать на весь мир.") );
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
        ch->pecho(_("Только подтвержденные богами персонажи могут общаться в этом канале.") );
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
            ch->pecho( localized( msgNochan, ch ) );
        else
            ch->pecho( _("Боги лишили тебя возможности общаться.") );
        
        return true;
    }

    if (IS_CHARMED(ch) && has_nochannel( ch->master )) {
        oldact(_("$c1 сдавленно хрипит, не в силах вымолвить ни слова."), ch, 0, 0, TO_ROOM);        
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
    
    oldact(_("$c1 пускает изо рта {Rр{Yа{Gз{Cн{Mо{Rц{Gв{Yе{Cт{Mн{Yы{Cе{x мыльные пузыри."), ch, 0, 0, TO_ROOM);
    oldact(_("Ты пускаешь изо рта {Rр{Yа{Gз{Cн{Mо{Rц{Gв{Yе{Cт{Mн{Yы{Cе{x мыльные пузыри."), ch, 0, 0, TO_CHAR);
    return true;
}

void GlobalChannel::triggers( Character *ch, const DLString &msg ) const
{
    if (dreamland->hasOption( DL_LOG_COMM ) 
        && getLog( ) != LOG_NEVER
        && (!ch->desc || ch->desc->echo))
    {
        LogStream::sendNotice( ) << "channel [" << getName( ) << "] " << ch->getNameP('1') << ": " << msg << endl;
    }
    
    if (!msg.empty() && hook) {
        DLString formatEn = localizedEn( msgOther );
        if (ooc)
            send_discord_ooc(ch, formatEn, msg);
        else
            send_discord_ic(ch, formatEn, msg);
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
        ch->pecho( _("Стены могилы поглощают звуки.") );
        return false;
    }
    
    if (ch->desc && !ch->desc->echo)
        return false;

    return true;
}


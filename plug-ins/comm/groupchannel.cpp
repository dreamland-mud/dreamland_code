/* $Id$
 *
 * ruffina, 2004
 */
#include "groupchannel.h"
#include "replay.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "loadsave.h"
#include "msgformatter.h"
#include "follow_utils.h"
#include "act.h"

#include "def.h"
#include "l10n.h"

/*-------------------------------------------------------------------------
 * GroupChannel
 *------------------------------------------------------------------------*/
const DLString GroupChannel::COMMAND_NAME = "gtell";

GroupChannel::GroupChannel( ) 
{
    name[EN] = COMMAND_NAME;
}

GroupChannel::~GroupChannel( )
{
}

void GroupChannel::run( Character *ch, const DLString &args )
{
    GlobalChannel::run(ch, args);
}

bool GroupChannel::saveCommand() const
{
    return CommandPlugin::saveCommand();
}

bool GroupChannel::isGlobalListener( Character *ch, Character *victim ) const
{
    if (!is_same_group( victim, ch ))
        return false;

    return GlobalChannel::isGlobalListener( ch, victim );
}

void GroupChannel::findListeners( Character *ch, Listeners &listeners ) const
{
    Character *gch;

    for (gch = char_list; gch != 0; gch = gch->next)
        if (isGlobalListener( ch, gch ))
            listeners.push_back( gch );
}

void GroupChannel::triggers(Character *ch, const DLString &msg) const
{
    GlobalChannel::triggers(ch, msg);

    // Ukrainian had no phrase of its own here, so a UA owner could not ask at all.
    if (!ch->is_npc() && (!str_prefix(msg.c_str(), "where are you?")
                          || !str_prefix(msg.c_str(), "где ты?")
                          || !str_prefix(msg.c_str(), "де ти?"))) {
        NPCharacter *pet = ch->getPC()->pet;

        if (!pet) {
            ch->pecho(_("На твой вопрос отвечать некому, у тебя нет {hh15питомца{x."));
            return;
        }

        if (IS_SET(ch->in_room->areaIndex()->area_flag, AREA_DUNGEON)) {
            ch->pecho(_("Здесь никто не откликнется на твой вопрос."));
            return;
        }

        // The frames are catalog-wrapped, but everything substituted into them
        // was Russian for every reader: the vocative the pet uses, and the zone
        // and room it names. Frame text is left untouched so the catalog keys
        // keep resolving.
        lang_t lang = viewerLang(ch);
        const char *master = GET_SEX(ch,
                                     lmsg(lang, "Master", "Хозяин", "Хазяїне"),
                                     lmsg(lang, "Master", "Хозяин", "Хазяїне"),
                                     lmsg(lang, "Mistress", "Хозяйка", "Хазяйко"));

        if (pet->position > POS_SLEEPING) {
            if (IS_AFFECTED(pet, AFF_BLIND)) {
                tell_raw(ch, pet, _("%s, я ничего не вижу!"), master);
            } else if (eyes_darkened(pet)) {
                tell_raw(ch, pet, _("%s, тут слишком темно, я ничего не вижу!"), master);
            } else {
                tell_raw(ch, pet, _("%s, я нахожусь в {hh%s{hx - %s"),
                         master,
                         pet->in_room->areaName(lang).c_str(), pet->in_room->getName(lang));
            }
        } else {
            ch->pecho(_("Твой питомец не в состоянии ответить на твой вопрос.")); // dumb wording, for debugging
        }
    }
}

bool GroupChannel::canTalkGlobally( Character *ch ) const
{
    if (!GlobalChannel::canTalkGlobally( ch ))
        return false;

    if (IS_SET( ch->comm, COMM_NOTELL )) {
        ch->pecho( _("Твое сообщение не получено!") );
        return false;
    }

    return true;
}

void GroupChannel::postOutput( Character *outputTo, const DLString &message ) const
{
    if (outputTo->getPC( ))
        remember_history_near( outputTo->getPC( ), message );
}


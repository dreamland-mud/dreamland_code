/* $Id$
 *
 * ruffina, 2004
 */
#include <list>

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
        // Every mob of the group answers: a pet, a charmie, a summon, a groupmate's
        // pet. Own followers count even when they were never grouped.
        list<NPCharacter *> mobs;

        for (Character *fch = char_list; fch != 0; fch = fch->next)
            if (fch->is_npc() && (fch->master == ch || is_same_group(fch, ch)))
                mobs.push_back(fch->getNPC());

        if (mobs.empty()) {
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

        for (auto &mob: mobs) {
            // A groupmate's pet has no call to name the asker its master.
            const char *addressee = mob->master == ch ? master : ch->getNameC();

            if (mob->position > POS_SLEEPING) {
                if (IS_AFFECTED(mob, AFF_BLIND)) {
                    tell_raw(ch, mob, _("%s, я ничего не вижу!"), addressee);
                } else if (eyes_darkened(mob)) {
                    tell_raw(ch, mob, _("%s, тут слишком темно, я ничего не вижу!"), addressee);
                } else {
                    tell_raw(ch, mob, _("%s, я нахожусь в {hh%s{hx - %s"),
                             addressee,
                             mob->in_room->areaName(lang).c_str(), mob->in_room->getName(lang));
                }
            } else {
                ch->pecho(_("%1$^C1 не в состоянии ответить на твой вопрос."), mob);
            }
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

DLString GroupChannel::webKind( ) const
{
    return "group";
}


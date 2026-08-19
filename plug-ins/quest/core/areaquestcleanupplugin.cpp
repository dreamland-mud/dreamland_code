#include <jsoncpp/json/json.h>

#include "areaquestcleanupplugin.h"
#include "xmlattributeareaquest.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "areaquest.h"
#include "areaquestutils.h"
#include "quest.h"
#include "player_utils.h"
#include "wiznet.h"
#include "configurable.h"
#include "descriptor.h"
#include "interp.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"
#include "l10n.h"

using namespace Scripting;

/** Shown to a newbie on entering the game: a one-line reminder of the questor
 *  task they left unfinished last session, so it isn't forgotten between logins.
 *  Older players know the `quest` command and don't need the nudge. */
static void remindEntryQuest( PCharacter *pch )
{
    if (!Player::isNewbie( pch ))
        return;

    Quest::Pointer quest = pch->getAttributes( ).findAttr<Quest>( "quest" );
    if (!quest)
        return;

    // shortInfo has no catcher above it, but it already runs from the web prompt
    // on every command, so it is safe to call here too.
    std::basic_ostringstream<char> buf;
    quest->shortInfo( buf, pch );
    if (buf.str( ).empty( ))
        return;
    buf << std::endl;

    pch->pecho(_("\r\n{YУ тебя есть незавершенное задание квестора:{x"));
    pch->send_to( buf );
    pch->pecho(_("Напиши {y{hcзадание{x, чтобы напомнить себе подробности."));
}

// A set of area quest settings defined in config/areaquest.json.
Json::Value aquestConfig;
CONFIGURABLE_LOADED(config, areaquest)
{
    aquestConfig = value;
}

// Auto-cancel area quests the player hasn't touched in `lifetime` days, so a
// forgotten one doesn't block taking new quests forever.
static void cleanupStaleAreaQuests( PCharacter *pch, ::Pointer<XMLAttributeAreaQuest> areaQuestAttr )
{
    int lifetime = aquestConfig["lifetime"].asInt();
    int cutoffTime = dreamland->getCurrentTime() - lifetime * Date::SECOND_IN_DAY;
    bool changed = false;

    for (auto &aquestDataPair: **areaQuestAttr) {
        const DLString &questId = aquestDataPair.first;
        AreaQuestData &aquestData = aquestDataPair.second;
        int latestInteraction = max(aquestData.timeupdate, aquestData.timestart);

        if (aquestData.questActive() && latestInteraction < cutoffTime) {

            AreaQuest *aquest = get_area_quest(questId);

            if (!aquest) {
                // Some weird non-existing quest in the attributes, just clean it.
                aquestData.cancel();
                wiznet(WIZ_QUEST, 0, 0, "Auto-cancelled obsolete area quest %s for %s", questId.c_str(), pch->getNameC());

            } else if (!aquest->flags.isSet(AQUEST_ONBOARDING|AQUEST_NOEXPIRE)) {
                // Make the player type 'quest cancel <num>', to allow onCancel triggers to run.
                pch->pecho(_("\r\n{yЗадание {Y%s{y будет отменено из-за неактивности.{x"), aquest->title.getForLang(viewerLang(pch)).c_str());

                interpret_raw(pch, "quest", "cancel %d", aquest->vnum.getValue());

                wiznet(WIZ_QUEST, 0, 0, "Auto-cancelled area quest %s for %s", questId.c_str(), pch->getNameC());
            }

            changed = true;
        }
    }

    if (changed)
        pch->save();
}

void AreaQuestCleanupPlugin::run( int oldState, int newState, Descriptor *d )
{
    Character *ch = d->character;

    if (!ch)
        return;

    if (newState != CON_PLAYING)
        return;

    PCharacter *pch = ch->getPC();
    auto areaQuestAttr = pch->getAttributes().findAttr<XMLAttributeAreaQuest>("areaquest");

    if (areaQuestAttr)
        cleanupStaleAreaQuests( pch, areaQuestAttr );

    // Independent of area quests: a newbie also gets reminded of their questor task.
    remindEntryQuest( pch );
}

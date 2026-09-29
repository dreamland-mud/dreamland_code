#include "clantasks.h"
#include "clanrecords.h"
#include "dlscheduler.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "dreamland.h"
#include "logstream.h"
#include "date.h"

/** How often online clan members get their tenure banked and ranks checked. */
static const int TENURE_PERIOD = 5 * 60;

void ClanTenureTask::run( )
{
    for (Descriptor *d = descriptor_list; d; d = d->next) {
        if (d->connected != CON_PLAYING || !d->character)
            continue;

        PCharacter *pc = d->character->getPC( );
        if (!pc)
            continue;

        clan_bank_tenure( pc );
        clan_promote_tenure( pc );
    }
}

int ClanTenureTask::getPriority( ) const
{
    return SCDP_AUTO + 20;
}

void ClanTenureTask::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( TENURE_PERIOD, Pointer( this ) );
}

void ClanDecayTask::run( )
{
    time_t now = dreamland->getCurrentTime( );
    int count = 0;

    for (auto &p: PCharacterManager::getPCM( ))
        if (clan_decay( p.second, now )) {
            PCharacterManager::saveMemory( p.second );
            count++;
        }

    if (count > 0)
        LogStream::sendNotice( ) << "Clan decay: " << count << " long-offline players lost tenure ranks." << endl;
}

int ClanDecayTask::getPriority( ) const
{
    // After PlayerLoadTask (SCDP_BOOT + 10) has filled the player list at boot.
    return SCDP_AUTO + 21;
}

void ClanDecayTask::after( )
{
    DLScheduler::getThis( )->putTaskInSecond( Date::SECOND_IN_DAY, Pointer( this ) );
}

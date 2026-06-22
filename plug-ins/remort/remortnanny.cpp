/* $Id: remortnanny.cpp,v 1.1.2.13.4.6 2009/01/01 14:13:18 rufina Exp $
 *
 * ruffina, 2004
 */

#include "remortnanny.h"
#include "logstream.h"
#include "class.h"
#include "room.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "merc.h"
#include "descriptor.h"
#include "interp.h"

#include "act.h"
#include "move_utils.h"
#include "vnum.h"
#include "def.h"


/**
 * Reset remort bonuses if they are no longer useful. If point imbalance is detected on login,
 * player will automatically get transferred to Jaga room.
 */
static void update_remort_bonuses(PCharacter *ch)
{
    int oldPoints = ch->getRemorts().points;

    for (int i = 0; i < stat_table.size; i++) {
        int max_stat = ch->getMaxTrain(i);
        int bonus_stat = ch->getRemorts().stats[i];
        int diff = max_stat + bonus_stat - MAX_STAT;

        if (diff > 0) {
            notice("Fixing remort bonus for %s: %s bonus was %d, current max is %d, diff %d",
                    ch->getName().c_str(), stat_table.name(i).c_str(), bonus_stat, max_stat, diff);
            ch->pecho("{cУ твоей расы теперь выше параметр '%s', тебе нет нужды покупать его дополнительно за реморты.{x",
                       stat_table.message(i).c_str());
            ch->getRemorts().stats[i] -= diff;
            ch->getRemorts().points += 10 * diff;
        }
    }

    if (oldPoints == 0 && ch->getRemorts().points != 0)
        ch->pecho("{cТы попадаешь в избушку к Бабе Яге, чтобы снова выбрать плюшки за реморты.{x");
}

/*-----------------------------------------------------------------------------
 * descriptor state listener for remorting players
 *----------------------------------------------------------------------------*/
void RemortNanny::run( int oldState, int newState, Descriptor *d )
{
    PCharacter *ch;
    Room *izba;

    if (newState != CON_PLAYING)
        return;

    if (!d->character || !( ch = d->character->getPC( ) ))
        return;

    if (ch->getRemorts( ).size( ) == 0)
        return;

    update_remort_bonuses(ch);

    if (ch->getRemorts( ).points == 0)
        return;

    if (!( izba = get_room_instance( ROOM_VNUM_REMORT ) )) {
        LogStream::sendError( ) << "Zero remort room!" << endl;
        return;
    }

    ch->position = std::max( POS_RESTING, (int)ch->position );

    if (ch->in_room != izba)
        transfer_char( ch, 0, izba,
                        "Ветер перемен подхватывает %1$C4 и уносит куда-то...",
                        "Ветер перемен переносит тебя в другое место...",
                        "%1$C1 влетает в избу через окошко.",
                        "Влетев в избу через окошко, ты шлепаешься на пол." );

    if (ch->pet && ch->pet->in_room != izba) {
        ch->pet->position = std::max( POS_RESTING, (int)ch->pet->position );
        transfer_char( ch->pet, 0, izba,
                        NULL, NULL, "%1$C1 влетает в избу через окошко." );
    }

    ch->save();
}

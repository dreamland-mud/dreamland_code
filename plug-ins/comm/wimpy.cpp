#include "pcharacter.h"
#include "commandtemplate.h"
#include "profession.h"
#include "merc.h"
#include "def.h"

PROF(samurai);

/*
 * 'Wimpy' originally by Dionysos.
 */
CMDRUNP( wimpy )
{
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ((ch->getProfession() == prof_samurai) && (ch->getRealLevel() >= 10))
    {
        ch->pecho("Стыдись! Это будет слишком большим позором для самурая.");
        if (ch->wimpy != 0) 
            ch->wimpy = 0;
        return;
    }

    if ( arg[0] == '\0' )
        wimpy = ch->max_hit / 5;
    else  wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
        ch->pecho("Твоя отвага превосходит твою мудрость.");
        return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
        ch->pecho("Это будет слишком большим позором для тебя.");
        return;
    }

    ch->wimpy        = wimpy;

    ch->pecho("Ты попытаешься убежать при %d очков жизни.", wimpy );
}




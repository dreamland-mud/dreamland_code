#include "commandtemplate.h"
#include "character.h"
#include "skillreference.h"
#include "interp.h"
#include "dl_strings.h"
#include "move_utils.h"
#include "merc.h"
#include "def.h"

GSN(fly);

CMDRUNP(flyup)
{
    interpret_raw(ch, "fly", "up");
}

CMDRUNP(flydown)
{
    interpret_raw(ch, "fly", "down");
}

CMDRUNP(fly)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg);

    if (!str_cmp(arg, "up") || !str_cmp(arg, "вверх"))
    {
        if (!can_fly(ch))
        {
            ch->pecho("Чтобы взлететь, найди крылья или зелье.");
            return;
        }

        if (!ch->posFlags.isSet(POS_FLY_DOWN))
        {
            ch->pecho("Ты уже летаешь.");
            return;
        }

        ch->posFlags.removeBit(POS_FLY_DOWN);
        ch->pecho("Ты начинаешь летать.");
        ch->recho("%^C1 начинает летать.", ch);
    }
    else if (!str_cmp(arg, "down") || !str_cmp(arg, "вниз"))
    {
        if (!is_flying(ch))
        {
            ch->pecho("Твои ноги уже на земле.");
            return;
        }

        ch->posFlags.setBit(POS_FLY_DOWN);
        ch->pecho("Твои ноги медленно опускаются на землю.");
        ch->recho("%^C1 медленно опускается на землю.", ch);
    }
    else
    {
        ch->pecho("Напиши {y{hcвзлететь{x или {y{hcнелетать{x.");
        return;
    }

    ch->setWait(gsn_fly->getBeats(ch));
}


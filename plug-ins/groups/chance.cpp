#include "chance.h"
#include "pcharacter.h"
#include "dl_math.h"
#include "wearloc_utils.h"

RELIG(taiphoen);

Chance::Chance(Character *ch, int effective, int maximum)
{
    this->ch = ch;
    this->effective = effective;
    this->maximum = maximum;
}

bool Chance::roll()
{
    return number_range(1, maximum) <= effective;
}

bool Chance::reroll()
{
    bool success = roll();

    if (success)
        return success;

    if (ch->is_npc())
        return success;

    // Reroll the dice for Taiphoen religion if a tattoo is worn, 50% chance.
    if (ch->getPC()->getReligion() != god_taiphoen)
        return success;

    if (chance(50))
        return success;

    Object *tattoo = get_eq_char(ch, wear_tattoo);
    if (tattoo == 0)
        return success;

    ch->pecho("%^O1 распахивает {Dпризрачные {Cкрылья{x над твоей головой, перемещая тебя на долю секунды назад во времени!", tattoo);
    ch->recho("%^O1 распахивает {Dпризрачные {Cкрылья{x над головой %C2, и реальность неуловимо меняется!", tattoo, ch);

    success = roll();
    if (success)
        ch->println("Ты пробуешь исправить свою ошибку... и {Gдобиваешься успеха!{x");
    else
        ch->println("Ты пробуешь исправить свою ошибку... но снова терпишь неудачу!{x");

    return success;
}


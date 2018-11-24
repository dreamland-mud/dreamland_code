#include "today.h"
#include "character.h"
#include "bonusflags.h"
#include "merc.h"
#include "def.h"

/*
 * Increase exp gained for killing on the 13th of each month.
 */
bool today_kill_bonus(Character *ch, const struct time_info_data &time_info)
{
    return time_info.day == 12 || ch->getReligion()->hasBonus(ch, RB_KILLEXP, time_info);
}

/*
 * Reduce mana costs on the 33th of every other month.
 */
bool today_mana_bonus(Character *ch, const struct time_info_data &time_info)
{
    return (time_info.day == 32 && (time_info.month % 2 == 0)) || ch->getReligion()->hasBonus(ch, RB_MANA, time_info);
}

/*
 * Better skills improve on the seventh of every month.
 */
bool today_learn_bonus(Character *ch, const struct time_info_data &time_info)
{
    return time_info.day == 6 || ch->getReligion()->hasBonus(ch, RB_LEARN, time_info);
}

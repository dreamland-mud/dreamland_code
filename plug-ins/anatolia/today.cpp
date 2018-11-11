#include "today.h"
#include "merc.h"

/*
 * Increase exp gained for killing on the 13th of each month.
 */
bool today_kill_bonus(const struct time_info_data &time_info)
{
    return time_info.day == 12;
}

/*
 * Reduce mana costs on the 33th of every other month.
 */
bool today_mana_bonus(const struct time_info_data &time_info)
{
    return time_info.day == 32 && (time_info.month % 2 == 0);
}

/*
 * Better skills improve on the seventh of every month.
 */
bool today_learn_bonus(const struct time_info_data &time_info)
{
    return time_info.day == 6;
}

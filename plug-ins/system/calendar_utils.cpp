#include "merc.h"

long day_of_epoch(int year, int month, int day)
{
    return year * 35 * 17 + month * 35 + day;
}

long day_of_epoch(const struct time_info_data &ti)
{
    return day_of_epoch(ti.year, ti.month, ti.day);
}


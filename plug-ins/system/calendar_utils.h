#ifndef CALENDAR_UTILS_H
#define CALENDAR_UTILS_H

struct time_info_data;

long day_of_epoch(int year, int month, int day);
long day_of_epoch(const struct time_info_data &ti);

#endif

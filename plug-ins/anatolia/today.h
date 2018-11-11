#ifndef TODAY_H
#define TODAY_H

struct time_info_data;

bool today_kill_bonus(const struct time_info_data &time_info);
bool today_mana_bonus(const struct time_info_data &time_info);
bool today_learn_bonus(const struct time_info_data &time_info);

#endif

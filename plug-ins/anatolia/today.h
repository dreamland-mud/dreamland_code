#ifndef TODAY_H
#define TODAY_H

struct time_info_data;
class Character;

bool today_kill_bonus(Character *ch, const struct time_info_data &time_info);
bool today_mana_bonus(Character *ch, const struct time_info_data &time_info);
bool today_learn_bonus(Character *ch, const struct time_info_data &time_info);

#endif

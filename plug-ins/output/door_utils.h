#ifndef DOOR_UTILS_H
#define DOOR_UTILS_H

extern const char *dir_name_big[];
extern const char **dir_name;
extern const char *dir_name_small[];
extern const char *ru_dir_name_small[];
extern const char *ru_dir_name_big[];


bool door_is_small(char c);
bool door_is_big(char c);
bool door_is_small_ru(char c);
bool door_is_big_ru(char c);

char door_translate_en_ru(char c);

#endif
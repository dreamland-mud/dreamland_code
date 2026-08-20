#include "door_utils.h"
#include "merc.h"
#include "def.h"

const char *dir_name_big[] = {"N","E","S","W","U","D"};
const char *dir_name_small[] = {"n","e","s","w","u","d"};
const char **dir_name = dir_name_small;
const char *ru_dir_name_big[] = {"С","В","Ю","З","П","О"};
const char *ru_dir_name_small[] = {"с","в","ю","з","п","о"};
// Ukrainian compass labels are two letters (Пн/Сх/Пд/Зх/Вг/Вн) on purpose: a
// single Ukrainian letter would collide with the movement aliases (typing "с"
// walks east/схід, not north). These are display-only -- never fed to
// find_door_in_array(), which keys on the first byte.
const char *ua_dir_name_big[] = {"Пн","Сх","Пд","Зх","Вг","Вн"};
const char *ua_dir_name_small[] = {"пн","сх","пд","зх","вг","вн"};

static int find_door_in_array(char c, const char * doors [])
{
    for (int d = 0; d < DIR_SOMEWHERE; d++)
        if (doors[d][0] == c)
            return d;

    return -1;
}

bool door_is_small(char c) 
{ 
    return find_door_in_array(c, dir_name_small) >= 0; 
}

bool door_is_big(char c) 
{ 
    return find_door_in_array(c, dir_name_big) >= 0; 
}

bool door_is_small_ru(char c) 
{ 
    return find_door_in_array(c, ru_dir_name_small) >= 0; 
}

bool door_is_big_ru(char c) 
{ 
    return find_door_in_array(c, ru_dir_name_big) >= 0; 
}

char door_translate_en_ru(char c)
{
    int d = find_door_in_array(c, dir_name_small);
    if (d >= 0)
        return ru_dir_name_small[d][0];
    
    d = find_door_in_array(c, dir_name_big);
    if (d >= 0)
        return ru_dir_name_big[d][0];

    return c;
}

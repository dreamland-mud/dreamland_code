#ifndef AREAQUESTUTILS_H
#define AREAQUESTUTILS_H

#include <list>
#include "dlstring.h"
#include "fenia/register-decl.h"

class Character;
class Object;
class Room;
class AreaQuest;
class Integer;

bool aquest_trigger(Character *mob, Character *ch, const DLString &trigType, const char *fmt, ...);
bool aquest_trigger(::Object *obj, Character *ch, const DLString &trigType, const char *fmt, ...);
bool aquest_trigger(Room *room, Character *ch, const DLString &trigType, const char *fmt, ...);

AreaQuest *get_area_quest(const DLString &questId);
AreaQuest *get_area_quest(const Integer& questId);
AreaQuest *get_area_quest(int questId);


#endif

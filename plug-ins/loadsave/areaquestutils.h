#ifndef AREAQUESTUTILS_H
#define AREAQUESTUTILS_H

#include <list>
#include "dlstring.h"
#include "fenia/register-decl.h"

class Character;
class PCharacter;
class Object;
class Room;
class AreaQuest;
class Integer;
class AreaQuestData;

bool aquest_trigger(Character *mob, Character *ch, const DLString &trigType, const char *fmt, ...);
bool aquest_trigger(::Object *obj, Character *ch, const DLString &trigType, const char *fmt, ...);
bool aquest_trigger(Room *room, Character *ch, const DLString &trigType, const char *fmt, ...);

DLString aquest_method_id(AreaQuest *q, int step, bool isBegin, const DLString &trigName);

AreaQuestData & aquest_data(PCharacter *ch, const DLString &questId);

AreaQuest *get_area_quest(const DLString &questId);
AreaQuest *get_area_quest(const Integer& questId);
AreaQuest *get_area_quest(int questId);

bool aquest_can_participate(PCharacter *ch, AreaQuest *q, const AreaQuestData &qdata);




#endif

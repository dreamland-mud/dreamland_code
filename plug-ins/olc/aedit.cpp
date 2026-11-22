/* $Id$
 *
 * ruffina, 2004
 */
#include <pcharacter.h>
#include <npcharacter.h>
#include <object.h>
#include <string.h>
#include <set>

#include "room.h"
#include "aedit.h"
#include "hedit.h"
#include "security.h"
#include "olc.h"
#include "areahelp.h"
#include "qedit.h"
#include "string_utils.h"
#include "websocketrpc.h"
#include "merc.h"
#include "arg_utils.h"
#include "areautils.h"
#include "update_areas.h"
#include "fight_extract.h"
#include "behavior_utils.h"
#include "save.h"
#include "dreamland.h"
#include "loadsave.h"
#include "wiznet.h"
#include "interp.h"
#include "act.h"

#include "def.h"

OLC_STATE(OLCStateArea);

OLCStateArea::OLCStateArea() : vnum( -1 ), area_flag(0, &area_flags)
{
    /*fromXML will!*/
}

OLCStateArea::OLCStateArea(AreaIndexData *original) : area_flag(0, &area_flags)
{
    if (original) {
        file_name = original->area_file->file_name;

        name = original->name;
        altname = original->altname;
        resetMessage = original->resetMessage;
        authors = original->authors;
        translator = original->translator;
        speedwalk = original->speedwalk;
        preposition = original->preposition;

        low_range = original->low_range;
        high_range= original->high_range;
        min_vnum  = original->min_vnum;
        max_vnum  = original->max_vnum;
        area_flag.setValue( original->area_flag );
        security  = original->security;
        vnum      = original->vnum;

        if (original->behavior) 
            try {
                ostringstream ostr;
                original->behavior.toStream( ostr );
                behavior.assign( ostr.str( ) );
            } catch (const ExceptionXMLError &e) {
                LogStream::sendError( ) << e.what( ) << endl;
            }
    }
    else {
        top_area++;
        vnum = top_area - 1;

        name[RU] = "Новая зона";
        security = 9;
        min_vnum = 0;
        max_vnum = 0;
        low_range = 0;
        high_range = 0;
        file_name.setValue( DLString( "area" ) + DLString( vnum ) + ".are" );
    }
}

OLCStateArea::~OLCStateArea() 
{
}

void OLCStateArea::commit() 
{
    AreaIndexData *original = AreaUtils::getByVnum(vnum);
    
    if(!original) {
        original = new AreaIndexData;
        original->area_file = new_area_file( file_name.getValue( ).c_str( ) );
        original->area_file->area = original;

        areaIndexes.push_back(original);

        original->create();
    }
    else {
        original->area_file->file_name = file_name;
    }

    original->name = name;
    original->altname = altname;
    original->authors = authors;
    original->translator = translator;
    original->speedwalk = speedwalk;
    original->resetMessage = resetMessage;
    original->preposition = preposition;
    original->low_range = low_range;
    original->high_range= high_range;
    original->min_vnum  = min_vnum;
    original->max_vnum  = max_vnum;
    original->area_flag = area_flag;
    original->security  = security;
    original->vnum = vnum;
    original->changed = true;

    if (!behavior.empty( )) 
        try {
            // A self-closing <behavior/> keeps the behavior the area already had.
            ::Pointer<AreaBehavior> previous( original->behavior.getPointer( ) );
            istringstream istr( behavior );
            original->behavior.fromStream( istr );
            if (original->behavior)
                original->behavior->setArea( original );
            else {
                LogStream::sendError( ) << "aedit " << original->getName( ) << ": behavior did not load, keeping the old one" << endl;
                original->behavior.setPointer( previous.getPointer( ) );
            }
        } catch (const Exception &e) {
            LogStream::sendError( ) << e.what( ) << endl;
        }

    // FIXME update all instances
    original->area->area_flag = area_flag;

    if (original->helps.empty()) {
        AreaHelp::Pointer help = AreaUtils::createHelp(original);
        XMLPersistent<HelpArticle> phelp(help.getPointer());
        original->helps.push_back(phelp);
    }
}

void OLCStateArea::statePrompt(Descriptor *d) 
{
    d->send( "Area> " );
}

bool OLCStateArea::checkOverlap(int lower, int upper)
{
    for(auto &pArea: areaIndexes) {
        if(pArea->vnum == vnum)
            continue;
            
        if ((lower <= pArea->min_vnum && pArea->min_vnum <= upper))
            return false;

        if ((lower <= pArea->max_vnum && pArea->max_vnum <= upper))
            return false;
    }
    return true;
}

DLString OLCStateArea::commitRefusal() const
{
    AreaIndexData *original = AreaUtils::getByVnum(vnum);
    int lower = min_vnum.getValue(), upper = max_vnum.getValue();

    for (area_file *f = area_file_list; f; f = f->next)
        if (f->area && f->area != original && f->file_name == file_name.getValue())
            return "Area " + DLString(f->area->vnum) + " already saves into " + file_name.getValue();

    // Only a new range is checked: an old area with strays outside its
    // range stays editable.
    if (!original || (lower == original->min_vnum && upper == original->max_vnum))
        return DLString::emptyString;

    // XMLArea::init saves only the prototypes inside the range.
    for (auto &r: original->roomIndexes)
        if (r.first < lower || r.first > upper)
            return "Room " + DLString(r.first) + " would fall outside vnums "
                   + DLString(lower) + "-" + DLString(upper);

    for (int i = 0; i < MAX_KEY_HASH; i++)
        for (MOB_INDEX_DATA *pMob = mob_index_hash[i]; pMob; pMob = pMob->next)
            if (pMob->area == original && !IS_SET(pMob->act, ACT_DELETED)
                && (pMob->vnum < lower || pMob->vnum > upper))
                return "Mobile " + DLString(pMob->vnum) + " would fall outside vnums "
                       + DLString(lower) + "-" + DLString(upper);

    for (int i = 0; i < MAX_KEY_HASH; i++)
        for (OBJ_INDEX_DATA *pObj = obj_index_hash[i]; pObj; pObj = pObj->next)
            if (pObj->area == original && !IS_SET(pObj->extra_flags, ITEM_DELETED)
                && (pObj->vnum < lower || pObj->vnum > upper))
                return "Object " + DLString(pObj->vnum) + " would fall outside vnums "
                       + DLString(lower) + "-" + DLString(upper);

    return DLString::emptyString;
}

DLString OLCStateArea::getJournalKind( ) const
{
    return "area";
}

int OLCStateArea::getJournalVnum( ) const
{
    return vnum.getValue();
}

void OLCStateArea::changed( PCharacter *ch )
{
    AreaIndexData *original = AreaUtils::getByVnum(vnum);    
    if (original)
        original->changed = true;
}

// Area Editor Functions.
AEDIT(show, "показать", "показать все поля")
{
    AreaIndexData *original = AreaUtils::getByVnum(vnum);
    Area *instance = original ? original->area : 0;

    ptc(ch, "Area:       [{C%5d{x]\n\r", vnum.getValue( ));
    ptc(ch, "Name EN:    [{W%s{x] %s\n\r", name.get(EN).c_str(), olc_edit_button(ch, "name", "web").c_str());   
    ptc(ch, "Name UA:    [{W%s{x] %s\n\r", name.get(UA).c_str(), olc_edit_button(ch, "uaname", "web").c_str());   
    ptc(ch, "Name RU:    [{W%s{x] %s\n\r", name.get(RU).c_str(), olc_edit_button(ch, "runame", "web").c_str());   
    ptc(ch, "Authors:    [{W%s{x]\n\r", authors.getValue( ).c_str( ));
    ptc(ch, "Translator: [{W%s{x]\n\r", translator.getValue( ).c_str( ));
    ptc(ch, "File:       [{W%s{x]\n\r", file_name.getValue( ).c_str( ));
    ptc(ch, "Vnums:      [{W%u-%u{x]\n\r", min_vnum.getValue( ), max_vnum.getValue( ));
    ptc(ch, "Levels:     [{W%u-%u{x]\n\r", low_range.getValue( ), high_range.getValue( ));
    ptc(ch, "Flags:      %s {D(? area_flags{w)\n\r",
        web_flag_menu(true, area_flags, "flags", area_flags.names(area_flag)).c_str());
    ptc(ch, "Security:   [{W%d{x]\n\r", security.getValue( ));
    ptc(ch, "Speedwalk:  EN [{W%s{x] %s  UA [{W%s{x] %s  RU [{W%s{x] %s\n\r", 
          String::stripEOL(speedwalk.get(EN)).c_str(), olc_edit_button(ch, "speedwalk", "web").c_str(),   
          String::stripEOL(speedwalk.get(UA)).c_str(), olc_edit_button(ch, "uaspeedwalk", "web").c_str(),   
          String::stripEOL(speedwalk.get(RU)).c_str(), olc_edit_button(ch, "ruspeedwalk", "web").c_str());   
    ptc(ch, "Preposition:EN [{W%s{x] %s  UA [{W%s{x] %s  RU [{W%s{x] %s\n\r",
          String::stripEOL(preposition.get(EN)).c_str(), olc_edit_button(ch, "preposition", "web").c_str(),
          String::stripEOL(preposition.get(UA)).c_str(), olc_edit_button(ch, "uapreposition", "web").c_str(),
          String::stripEOL(preposition.get(RU)).c_str(), olc_edit_button(ch, "rupreposition", "web").c_str());
    ptc(ch, "Message:    EN [{W%s{x] %s  UA [{W%s{x] %s  RU [{W%s{x] %s\n\r",
          String::stripEOL(resetMessage.get(EN)).c_str(), olc_edit_button(ch, "message", "web").c_str(),   
          String::stripEOL(resetMessage.get(UA)).c_str(), olc_edit_button(ch, "uamessage", "web").c_str(),   
          String::stripEOL(resetMessage.get(RU)).c_str(), olc_edit_button(ch, "rumessage", "web").c_str());   

    if (instance)
        ptc(ch, "{DMain instance: empty [%s], age [%d], players [%d], flags [%s], rooms count [%d]{x\n\r",
            (instance->empty ? "yes" : "no"), instance->age, instance->nplayer, 
            area_flags.names(instance->area_flag).c_str(), instance->rooms.size());
                     
    if (!behavior.empty( ))
        ptc(ch, "Behavior:\r\n%s", behavior.c_str( ));

    if (original) {
        DLString buf;

        for (auto &article: original->helps) 
            if (article->getID() > 0)
                buf += "    [{C" + article->getAllKeywordsString() + "{x] "
                    +  olc_edit_button(ch, "hedit", DLString(article->getID()))
                    + " {D(hedit " + DLString(article->getID()) + "){x\r\n";

        ch->pecho("Helps:\r\n" + buf);

    } else {
        ptc(ch, "Helps:      {Dno helps for the new area, save first\r\n");
    }

    if (original) {
        ostringstream buf;

        const DLString lineFormatQuestEdit =
            "{W[" + web_cmd(ch, "qedit $1", "%6d") + "{W] {c%s{x, {C%d{x step(s), levels {C%d{x-{C%d{x\r\n";

        for (auto &q: original->quests) {
            buf << fmt(0, lineFormatQuestEdit.c_str(),
                    q->vnum.getValue(),
                    q->title.get(LANG_DEFAULT).c_str(),
                    q->steps.size(),
                    q->minLevel.getValue(), q->maxLevel.getValue());
        }

        if (!buf.str().empty())
            ch->pecho("Quests:\r\n" + buf.str());
        else
            ch->pecho("Quests:     (none) {D({y{hcquest create{x{D){x\r\n");

    } else {
        ptc(ch, "Quests:     {Dno quests for the new area, save and use {yquest create{x\r\n");
    }

    return false;
}

static AreaQuest * arg_areaquest(const DLString &arg, AreaIndexData *pArea)
{
    Integer questId;

    if (arg.isNumber() && Integer::tryParse(questId, arg)) {
        auto q = pArea->questMap.find(questId);
        if (q != pArea->questMap.end())        
            return q->second;
    }

    return 0;
}

int OLCStateArea::nextQuestVnum(AreaIndexData *original, const std::set<int> *skip)
{
    for (int v = original->min_vnum; v <= original->max_vnum; v++)
        if (v > 0 && areaQuests.find(v) == areaQuests.end() && (!skip || !skip->count(v)))
            return v;

    return -1;
}

AreaQuest * OLCStateArea::questCreate(PCharacter *ch, AreaIndexData *original, const DLString &argVnum)
{
    Integer vnum;

    // 'quest create' - assign next free vnum
    if (argVnum.empty()) {
        vnum = nextQuestVnum(original);

        if (vnum < 0) {
            ptc(ch, "В зоне не осталось свободного внума для квеста.\r\n");
            return 0;
        }

    } // 'quest create <vnum>' - check if vnum is valid
    else if (!argVnum.isNumber() || !Integer::tryParse(vnum, argVnum)) {
        ptc(ch, "Формат: quest create [номер]\r\n");
        return 0;
    }

    if (vnum < original->min_vnum || vnum > original->max_vnum) {
        ptc(ch, "Номер квеста должен лежать в диапазоне от %d до %d.\r\n",
            original->min_vnum, original->max_vnum);
        return 0;
    }

    // Quest vnums are global: a second quest under the same number would
    // replace the first one in areaQuests and orphan it.
    if (areaQuests.find(vnum) != areaQuests.end()) {
        ptc(ch, "Квест %d уже существует.\r\n", vnum.getValue());
        return 0;
    }

    AreaQuest::XMLPointer newQuest(NEW);
    newQuest->vnum.setValue(vnum);
    newQuest->pAreaIndex = original;

    original->questMap[vnum] = *newQuest;
    original->quests.push_back(newQuest);
    areaQuests[vnum] = *newQuest;
    original->changed = true;

    return *newQuest;
}

bool OLCStateArea::questDelete(PCharacter *ch, AreaIndexData *original, const DLString &arg)
{
    AreaQuest *q = arg_areaquest(arg, original);

    if (!q) {
        ptc(ch, "Квест по номеру %s не определен.\r\n", arg.c_str());
        return false;
    }

    original->questMap.erase(q->vnum.getValue());
    areaQuests.erase(q->vnum.getValue());
    original->quests.remove(q);
    original->changed = true;

    ptc(ch, "Квест удален!\r\n");
    return true;
}

AEDIT(quest, "квест", "редактировать квесты в зоне")
{
    AreaIndexData *original = AreaUtils::getByVnum(vnum);
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (!original) {
        ptc(ch, "No quests for the new area, save and use {yquest create{x\r\n");
        return false;
    }

    // 'quest create' 'quest create 2300'
    if (arg_is(arg, "create")) {
        AreaQuest *newQuest = questCreate(ch, original, args.getOneArgument());
        if (!newQuest)
            return false;

        OLCStateAreaQuest::Pointer qedit(NEW, newQuest);
        qedit->attach(ch);
        qedit->show(ch);
        return true;
    }   

    // 'quest delete 2300'
    if (arg_is(arg, "del"))
        return questDelete(ch, original, args.getOneArgument());

    // 'quest 2300'
    if (arg.isNumber()) {
        AreaQuest *q = arg_areaquest(arg, original);

        if (!q) {
            ptc(ch, "Квест по номеру %s не определен.\r\n", arg.c_str());
            return false;
        }

        OLCStateAreaQuest::Pointer qedit(NEW, q);
        qedit->attach(ch);
        qedit->show(ch);
        return true;
    }

    ptc(ch, "Формат: quest <vnum>, quest create [<vnum>], quest delete <vnum>\r\n");
    return false;
}

/*
 * 'reset purge': what a fresh reset of a room may sweep away. A mob goes only
 * when a reset of this area can bring its prototype back; it stays when a
 * player is its master, leader or rider, when it is NOPURGE or switched into,
 * or when anything it carries is kept by the object rule. An object stays
 * when it is a PC corpse, owned, limited, NOPURGE, or holds such an object.
 * Masterless player summons with unowned gear stay only through the first
 * rule; ownerless floor stashes go.
 */
static bool reset_purge_keeps_obj(Object *obj)
{
    if (obj->item_type == ITEM_CORPSE_PC
        || !obj->getOwner().empty()
        || obj->pIndexData->limit != -1
        || IS_OBJ_STAT(obj, ITEM_NOPURGE))
        return true;

    for (Object *in = obj->contains; in; in = in->next_content)
        if (reset_purge_keeps_obj(in))
            return true;

    return false;
}

/** Mob prototypes an 'M' reset in this area loads. */
static std::set<int> reset_purge_mob_vnums(AreaIndexData *pArea)
{
    std::set<int> vnums;

    for (auto &r: pArea->roomIndexes)
        for (auto &pReset: r.second->resets)
            if (pReset->command == 'M')
                vnums.insert(pReset->arg1);

    return vnums;
}

static bool reset_purge_keeps_mob(Character *rch, const std::set<int> &resetMobs)
{
    if (!rch->is_npc() || rch->desc != 0 || IS_SET(rch->act, ACT_NOPURGE))
        return true;

    // Nothing brings it back: a summon, a quest mob, a wanderer from elsewhere.
    if (!resetMobs.count(rch->getNPC()->pIndexData->vnum))
        return true;

    if ((rch->master && !rch->master->is_npc())
        || (rch->leader && !rch->leader->is_npc())
        || (rch->mount && !rch->mount->is_npc()))
        return true;

    for (Object *obj = rch->carrying; obj; obj = obj->next_content)
        if (reset_purge_keeps_obj(obj))
            return true;

    return false;
}

/** Saving of mobs and objects off for the sweep, back on however it ends. */
struct ResetPurgeNoSave {
    ResetPurgeNoSave() {
        dreamland->removeOption(DL_SAVE_MOBS);
        dreamland->removeOption(DL_SAVE_OBJS);
    }
    ~ResetPurgeNoSave() {
        dreamland->resetOption(DL_SAVE_MOBS);
        dreamland->resetOption(DL_SAVE_OBJS);
    }
};

/** A room with a player in it, linkdead or switched into a mob. */
static bool reset_purge_has_player(Room *room)
{
    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        if (!rch->is_npc() || rch->desc != 0)
            return true;
    return false;
}

static void reset_purge_room(Room *room, const std::set<int> &resetMobs, int &mobs, int &objs)
{
    // Candidates first, extraction after: a mob's or item's scripts may
    // extract or recycle another one while the room lists change under us.
    vector<Character *> victims;
    vector<ExtractWatch> watches;

    for (Character *rch = room->people; rch; rch = rch->next_in_room)
        if (!reset_purge_keeps_mob(rch, resetMobs))
            victims.push_back(rch);

    for (auto &v: victims)
        watches.push_back(ExtractWatch(v, true));

    for (size_t i = 0; i < victims.size(); i++)
        if (!watches[i].gone() && victims[i]->in_room == room) {
            extract_char(victims[i]);
            mobs++;
        }

    // Collected after the mobs: what they dropped is judged too.
    vector<pair<Object *, long long> > items;

    for (Object *obj = room->contents; obj; obj = obj->next_content)
        if (!reset_purge_keeps_obj(obj))
            items.push_back(make_pair(obj, obj->getID()));

    for (auto &it: items) {
        Object *obj = it.first;
        if (obj->extracted || obj->in_room != room || obj->getID() != it.second)
            continue;
        extract_obj(obj);
        objs++;
    }
}

AEDIT(reset, "сбросить", "сбросить арию, обновив всех мобов, предметы и двери; reset purge -- с очисткой комнат без игроков")
{
    AreaIndexData *original = AreaUtils::getByVnum(vnum);

    if (!original) {
        stc("Создание арии не завершено - нечего сбрасывать.\n\r", ch);
        return false;
    }

    if (!arg_oneof_strict(argument, "purge", "очистить")) {
        // FIXME reset either all instances or the current one.
        reset_area(original->area, FRESET_ALWAYS);
        stc("Ария сброшена.\n\r", ch);
        return false;
    }

    int skipped = 0, mobs = 0, objs = 0;
    list<Room *> purged;
    std::set<int> resetMobs = reset_purge_mob_vnums(original);

    {
        ResetPurgeNoSave noSave;

        for (auto &r: original->area->rooms) {
            Room *room = r.second;

            if (reset_purge_has_player(room)) {
                skipped++;
                continue;
            }

            reset_purge_room(room, resetMobs, mobs, objs);
            purged.push_back(room);
        }
    }

    for (auto &room: purged) {
        save_items(room);
        save_mobs(room);
    }

    reset_area(original->area, FRESET_ALWAYS);

    wiznet(WIZ_LOAD, WIZ_SECURE, ch->get_trust(), "%C1 purges and resets area %d.", ch, original->vnum);
    LogStream::sendNotice() << ch->getNameC() << " aedit reset purge area " << original->vnum
                            << ": " << mobs << " mobs, " << objs << " objects, "
                            << skipped << " rooms with players skipped" << endl;

    ptc(ch, "Ария очищена и сброшена. Убрано мобов: %d, предметов: %d. Пропущено комнат с игроками: %d.\n\r",
        mobs, objs, skipped);
    return false;
}

AEDIT(create, "создать", "создать новую арию")
{
    OLCStateArea::Pointer ae(NEW, (AreaIndexData *)NULL);
    ae->attach(ch);
    ae->findCommand(ch, "show")->entryPoint(ch, "");

    stc("Ария создана.\n\r", ch);
    return false;
}

AEDIT(name, "имя", "установить имя зоны")
{
    return editor(argument, name[EN], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(uaname, "укимя", "установить имя зоны")
{
    return editor(argument, name[UA], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(runame, "ruимя", "установить имя зоны")
{
    return editor(argument, name[RU], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(message, "сообщение", "установить сообщение, видимое при сбросе зоны")
{
    return editor(argument, resetMessage[EN], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(uamessage, "уксообщение", "установить сообщение, видимое при сбросе зоны")
{
    return editor(argument, resetMessage[UA], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(rumessage, "русообщение", "установить сообщение, видимое при сбросе зоны")
{
    return editor(argument, resetMessage[RU], (editor_flags)(ED_NO_NEWLINE|ED_UPPER_FIRST_CHAR));
}

AEDIT(speedwalk, "маршрут", "установить маршрут, как добраться от Рыночной Площади")
{
    return editor(argument, speedwalk[EN], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(uaspeedwalk, "укмаршрут", "установить маршрут, как добраться от Рыночной Площади")
{
    return editor(argument, speedwalk[UA], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(ruspeedwalk, "румаршрут", "установить маршрут, как добраться от Рыночной Площади")
{
    return editor(argument, speedwalk[RU], (editor_flags)(ED_NO_NEWLINE));
}

/* Goes in front of the zone name in the 6th (locative) case: "in Midgaard",
 * "on the Chessboard". English keeps its article here, hence a free-form string
 * and not a flag. */
AEDIT(preposition, "предлог", "установить предлог перед названием зоны: in, on the")
{
    return editor(argument, preposition[EN], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(uapreposition, "укпредлог", "установить предлог перед названием зоны: в, у, на")
{
    return editor(argument, preposition[UA], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(rupreposition, "рупредлог", "установить предлог перед названием зоны: в, во, на")
{
    return editor(argument, preposition[RU], (editor_flags)(ED_NO_NEWLINE));
}

AEDIT(flags, "флаги", "установить или сбросить флаги арии (? area_flags)")
{
    return flagBitsEdit(area_flag);
}

AEDIT(file, "файл", "установить имя файла, в который сохраняется ария")
{
    char file[MAX_STRING_LENGTH];
    int i, length;

    if (ch->getSecurity( ) < 10) {
        stc("Недостаточно прав для изменения имени файла арии.\r\n", ch);
        return false;
    }

    one_argument(argument, file);        /* Forces Lowercase */

    if (!*argument) {
        stc("Синтаксис:  filename [$имя_файла]\n\r", ch);
        return false;
    }

    /*
     * Simple Syntax Check.
     */
    length = strlen(argument);
    if (length > 64) {
        stc("No more than eight characters allowed.\n\r", ch);
        return false;
    }

    /*
     * Allow only letters and numbers.
     */
    for (i = 0; i < length; i++) {
        if (!isalnum(file[i])) {
            stc("Only letters and numbers are valid.\n\r", ch);
            return false;
        }
    }

    strcat(file, ".are");
    file_name = file;

    stc("Filename set.\n\r", ch);
    return true;
}

AEDIT(security, "права", "установить уровень доступа к арии, 0..9")
{
    char sec[MAX_STRING_LENGTH];
    int value;

    one_argument(argument, sec);

    if (!is_number(sec) || !*sec) {
        stc("Syntax:  security [#xlevel]\n\r", ch);
        return false;
    }

    value = atoi(sec);

    if (value > ch->getSecurity( ) || value < 0) {
        if (ch->getSecurity() != 0) {
            ch->pecho("Security is 0-%d.", ch->getSecurity());
        }
        else
            stc("Security is 0 only.\n\r", ch);
        return false;
    }

    security = value;

    stc("Security set.\n\r", ch);
    return true;
}

AEDIT(authors, "авторы", "установить имена создателей арии или none")
{
    if (!*argument) {
        stc("Syntax:  authors names\n\r", ch);
        stc("Syntax:  authors none\n\r", ch);
        return false;
    }

    if (arg_is_clear(argument))
        authors = "";
    else
        authors = argument;

    return true;
}

AEDIT(translator, "переводчик", "установить имена переводчиков или none")
{
    if (!*argument) {
        stc("Syntax:  translator names\n\r", ch);
        stc("Syntax:  translator none\n\r", ch);
        return false;
    }

    if (arg_is_clear(argument))
        translator = "";
    else
        translator = argument;

    return true;
}


AEDIT(vnums, "внумы", "устаноить диапазон внумов (нижний верхний)")
{
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }
    
    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || !*lower || !is_number(upper) || !*upper) {
        stc("Syntax:  vnum [#xlower] [#xupper]\n\r", ch);
        return false;
    }

    ilower = atoi(lower);
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    min_vnum = ilower;
    max_vnum = iupper;
    stc("Lower & Upper vnums set.\n\r", ch);    
    return true;
}

AEDIT(lvnum, "нвнум", "установить нижнюю границу диапазона внумов")
{
    char lower[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }

    one_argument(argument, lower);

    if (!is_number(lower) || !*lower) {
        stc("Syntax:  min_vnum [#xlower]\n\r", ch);
        return false;
    }
    
    ilower = atoi(lower);
    iupper = max_vnum;

    if (ilower > iupper) {
        stc("OLCStateArea:  Value must be less than the max_vnum.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    min_vnum = ilower;
    stc("Lower vnum set.\n\r", ch);
    return true;
}

AEDIT(uvnum, "ввнум", "установить верхнюю границу диапазона внумов")
{
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    if (ch->getSecurity() < 10) {
        stc("Недостаточно прав для изменения внумов арии.\r\n", ch);
        return false;
    }

    one_argument(argument, upper);

    if (!is_number(upper) || !*upper) {
        stc("Syntax:  max_vnum [#xupper]\n\r", ch);
        return false;
    }
    
    ilower = min_vnum;
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    if (!checkOverlap(ilower, iupper)) {
        stc("OLCStateArea:  Range must include only this area.\n\r", ch);
        return false;
    }

    max_vnum = iupper;
    stc("Upper vnum set.\n\r", ch);
    return true;
}

AEDIT(levels, "уровни", "установить рекомендуемые уровни арии (нижний верхний)")
{
    char lower[MAX_STRING_LENGTH];
    char upper[MAX_STRING_LENGTH];
    int ilower;
    int iupper;

    argument = one_argument(argument, lower);
    one_argument(argument, upper);

    if (!is_number(lower) || !*lower || !is_number(upper) || !*upper) {
        stc("Syntax:  levels [#xlower] [#xupper]\n\r", ch);
        return false;
    }
    
    ilower = atoi(lower);
    iupper = atoi(upper);
    
    if (ilower > iupper) {
        stc("OLCStateArea:  Upper must be larger then lower.\n\r", ch);
        return false;
    }

    low_range = ilower;
    high_range = iupper;
    stc("Level range set.\n\r", ch);
    return true;
}

AEDIT(behavior, "поведение", "запустить строковый редактор для установки поведения (XML)")
{
    if (argument[0] == '\0') {
        if(!sedit(behavior))
            return false;

        stc("Behavior set.\n\r", ch);
        return true;
    }

    if (arg_is_clear(argument)) {
        behavior.clear( );
        stc("Поведение очищено.\r\n", ch);
        return true;
    }

    stc("Syntax:  behavior\n\r", ch);
    stc("Syntax:  behavior clear\n\r", ch);
    return false;
}

AEDIT(commands, "команды", "показать список встроенных команд edit")
{
    do_commands(ch);
    return false;
}

AEDIT(done, "готово", "выйти из редактора (не забывайте про asave changed)") 
{
    commit();
    detach(ch);
    return true;
}

AEDIT(cancel, "отменить", "отменить все изменения и выйти из редактора")
{
    detach(ch);
    return false;
}

AEDIT(dump, "вывод", "(отладка) вывести внутреннее состояние редактора")
{
    ostringstream os;
    XMLStreamable<OLCState> xs( "OLCState" );
    
    xs.setPointer( this);
    xs.toStream(os);

    stc(os.str() + "\r\n", ch);
    return false;
}

CMD(aedit, 50, "", POS_DEAD, 103, LOG_ALWAYS, 
        "Online area editor.")
{
    AreaIndexData *pArea;
    int value;
    char arg[MAX_STRING_LENGTH];

    pArea = ch->in_room->areaIndex();

    argument = one_argument(argument, arg);
    if (is_number(arg)) {
        value = atoi(arg);
        pArea = AreaUtils::getByVnum(value);
        if (!pArea) {
            stc("That area vnum does not exist.\n\r", ch);
            return;
        }
    }
    else {
        if (arg_is_strict(arg, "create")) {
            if (ch->getSecurity() < 10) {
                stc("Insuficiente seguridad para crear areas.\n\r", ch);
                return;
            }
            OLCStateArea::Pointer ae(NEW, (AreaIndexData *)NULL);
            ae->attach(ch);
            ae->findCommand(ch, "show")->entryPoint(ch, "");
            stc("Ария создана.\r\n", ch);
            return;
        }
    }

    if (!OLCState::can_edit(ch, pArea)) {
        stc("У тебя недостаточно прав для редактирования арий.\n\r", ch);
        return;
    }

    OLCStateArea::Pointer ae(NEW, pArea);
    ae->attach(ch);
    ae->findCommand(ch, "show")->entryPoint(ch, "");
}


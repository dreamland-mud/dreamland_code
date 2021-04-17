#include "feniatriggers.h"
#include "wrapperbase.h"
#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "plugininitializer.h"
#include "wrappermanager.h"
#include "iconvmap.h"
#include "defaultspell.h"
#include "defaultaffecthandler.h"
#include "feniaskillaction.h"
#include "websocketrpc.h"
#include "dlfileloader.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "damageflags.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

static IconvMap utf2koi("utf-8", "koi8-r");

static const DLString EXAMPLE_FOLDER = "fenia.examples";

using namespace Scripting;

FeniaTriggerLoader *feniaTriggers = 0;

FeniaTriggerLoader::FeniaTriggerLoader()
{
    checkDuplicate( feniaTriggers );
    feniaTriggers = this;
}
FeniaTriggerLoader::~FeniaTriggerLoader()
{
    feniaTriggers = 0;
}

void FeniaTriggerLoader::initialization()
{
    loadFolder("obj");
    loadFolder("mob");
    loadFolder("room");
    loadFolder("spell");
    loadFolder("affect");
}

void FeniaTriggerLoader::destruction()
{

}

void FeniaTriggerLoader::loadFolder(const DLString &indexType)
{
    DLFileLoader loader(EXAMPLE_FOLDER + "/" + indexType, "");
    loader.loadAll();

    DLFileLoader::Files::const_iterator f;
    DLFileLoader::Files files = loader.getAll();

    TriggerContent triggers;
    for (f = files.begin(); f != files.end(); f++) {
        // Assume that the source file is in utf-8.
        triggers[f->first] = DLString(utf2koi(f->second.content));
    }

    indexTriggers[indexType] = triggers;
}

// Return 'Use' for 'onUse' or 'postUse'.
DLString triggerType(const DLString &name) 
{
    DLString key = name;
    static const DLString ON("on");
    static const DLString POST("post");

    if (ON.strPrefix(key))
        key = key.substr(ON.size());
    else if (POST.strPrefix(key))
        key = key.substr(POST.size());

    return key;
}

// Return true for strings like Aaaaa.
bool stringIsCapitalized(const DLString &str)
{
    return !str.empty() && dl_isupper(str.at(0));
}

void FeniaTriggerLoader::showAvailableTriggers(PCharacter *ch, const DLString &indexType) const
{
    IndexTriggers::const_iterator i = indexTriggers.find(indexType);
    if (i == indexTriggers.end())
        return;

    ostringstream buf;
    buf << "Доступные триггера (команда fenia <trig>): ";

    const TriggerContent &triggers = i->second;
    for (TriggerContent::const_iterator t = triggers.begin(); t != triggers.end(); t++) {
        buf <<  web_cmd(ch, "fenia $1", t->first) << " ";
    }

    buf << endl;
    ch->send_to(buf);
}

void FeniaTriggerLoader::showAssignedTriggers(PCharacter *ch, Scripting::Object *wrapper) const
{
    WrapperBase *base = get_wrapper(wrapper);
    if (!base)
        return;

    StringSet triggers, misc;
    ostringstream buf;

    base->collectTriggers(triggers, misc);
    if (!triggers.empty()) {
        buf << "{gFenia triggers{x:           ";
        for (auto &t: triggers)
            buf << web_cmd(ch, "fenia $1", t) << " ";
        buf << endl;    
    }

    if (!misc.empty()) {
        buf << "{gFenia fields and methods{x: ";
        for (auto &t: misc)
            buf << web_cmd(ch, "fenia $1", t) << " ";
        buf << endl;    
    }

    ch->send_to(buf);
}

bool FeniaTriggerLoader::clearTrigger(Scripting::Object *wrapper, const DLString &trigName) const
{
    if (!wrapper)
        return false;

    WrapperBase *base = get_wrapper(wrapper);
    if (!base)
        return false;

    Scripting::IdRef methodId(trigName);
    if (base->getField(methodId).type == Register::NONE)
        return false;

    base->setField(methodId, Register());
    return true;
}

static void show_one_trigger(PCharacter *ch, DefaultSpell *spell, const char *trigName, bitnumber_t target_mask, ostringstream &buf)
{
    bool hasTrigger = FeniaSkillActionHelper::spellHasTrigger(spell, trigName);
    bool hasTarget = spell->target.isSet(target_mask);

    if (hasTarget || hasTrigger)
    {
        buf << "{g" << web_cmd(ch, "fenia $1", trigName) 
            << (hasTrigger ? "{g*{x" : "{x")
            << "  ";
    }
}

void FeniaTriggerLoader::showAvailableTriggers(PCharacter *ch, DefaultSpell *spell) const
{
    ostringstream buf;

    show_one_trigger(ch, spell, "runVict", TAR_CHAR_ROOM|TAR_CHAR_SELF|TAR_CHAR_WORLD, buf);
    show_one_trigger(ch, spell, "runObj", TAR_OBJ_EQUIP|TAR_OBJ_INV|TAR_OBJ_ROOM|TAR_OBJ_WORLD, buf);
    show_one_trigger(ch, spell, "runRoom", TAR_PEOPLE|TAR_ROOM, buf);
    show_one_trigger(ch, spell, "runArg", TAR_IGNORE|TAR_CREATE_MOB|TAR_CREATE_OBJ, buf);

    if (buf.str().empty())
        buf << "(укажи цель заклинания)";
    else
        buf << "{D(fenia <trig> [clear]){x";
        
    buf << endl;
    ch->send_to(buf);
}

void FeniaTriggerLoader::showAvailableTriggers(PCharacter *ch, DefaultAffectHandler *ah) const
{
    ostringstream buf;

    IndexTriggers::const_iterator i = indexTriggers.find("affect");
    if (i == indexTriggers.end())
        return;

    WrapperBase *wrapper = ah->getWrapper();    

    const TriggerContent &triggers = i->second;
    for (TriggerContent::const_iterator t = triggers.begin(); t != triggers.end(); t++) {
        IdRef methodId(t->first);
        Register method;
        bool hasTrigger = wrapper && wrapper->triggerFunction(methodId, method);
        buf << web_cmd(ch, "fenia $1", t->first)
            << (hasTrigger ? "{g*" : "")
            << "{x ";
    }

    buf << "{D(fenia <trig> [clear]){x" << endl;
    ch->send_to(buf);
}

static Register get_wrapper_for_index_data(int vnum, const DLString &type)
{
    Register w;
    if (type == "obj") {
        w = WrapperManager::getThis()->getWrapper(get_obj_index(vnum));
    } else if (type == "room") {
        w = WrapperManager::getThis()->getWrapper(get_room_instance(vnum));
    } else if (type == "mob") {
        w = WrapperManager::getThis()->getWrapper(get_mob_index(vnum));
    }
    return w;
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, XMLIndexData &indexData, const DLString &constArguments) const
{
    if (!is_websock(ch)) {
        ch->pecho("Эта крутая фишка доступна только в веб-клиенте.");
        return false;
    }

    Register w = get_wrapper_for_index_data(indexData.getVnum(), indexData.getIndexType());
    if (w.type == Register::NONE)
        return false;
        
    WrapperBase *base = get_wrapper(w.toObject());
    if (!base) {
        ch->pecho("Не могу найти wrapper base, всё плохо.");
        return false;
    }

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Scripting::IdRef methodId(methodName);
    Register retval = base->getField(methodId);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        DLString trigType = triggerType(methodName);
        if (trigType == methodName) {
            ch->pecho("Название триггера должно начинаться с 'on' или 'post'.");
            return false;
        }

        if (!stringIsCapitalized(trigType)) {
            ch->pecho("Название триггера должно выглядеть так: onFight, postSpeech, onWear и т.п.");
            return false;
        }

        std::vector<DLString> parms(2);
        // Create codesource subject.
        parms[0] = dlprintf("areas/%s/%s/%d.%s", 
                        indexData.getArea()->area_file->file_name, 
                        indexData.getIndexType(),
                        indexData.getVnum(),
                        methodName.c_str());   

        // Create codesource body with example code.
        DLString tmpl;
        if (!findExample(ch, methodName, indexData.getIndexType(), tmpl))
            return false;

        tmpl.replaces("@vnum@", indexData.getVnum());
        tmpl.replaces("@trig@", methodName);
        parms[1] = tmpl;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для нового сценария, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}

// Look up trigger example for given index type and method name.
bool FeniaTriggerLoader::findExample(Character *ch, const DLString &methodName, const DLString &indexType, DLString &tmpl) const
{
    IndexTriggers::const_iterator i = indexTriggers.find(indexType);
    if (i == indexTriggers.end()) {
        ch->pecho("Нет ни одного триггера, попросите богов принять меры.");
        return false;
    }

    TriggerContent::const_iterator t = i->second.find(methodName);
    if (t == i->second.end()) {
        ch->printf("Триггер %s не найден, проверьте написание или попросите богов добавить его.\r\n", methodName.c_str());
        return false;
    }

    tmpl = t->second;
    return true;
}

// Launch web editor with the codesource for this function.
bool FeniaTriggerLoader::editExisting(Character *ch, Register &retval) const
{
    // Fenia field is not a function.
    if (retval.type != Register::FUNCTION) {
        ch->pecho("Это поле уже задано, но это не функция. Попробуйте что-то еще.");
        return false;
    }

    Scripting::CodeSourceRef csRef = retval.toFunction()->getFunction()->source;

    // Construct cs_edit arguments: subject, editor content and line number.
    std::vector<DLString> parms(3);
    parms[0] = csRef.source->name;
    parms[1] = csRef.source->content;
    parms[2] = csRef.line; // TODO line number at the end of the function

    // Open the editor.
    ch->desc->writeWSCommand("cs_edit", parms);
    ch->printf("Запускаю веб-редактор для сценария %s, строка %d.\r\n", csRef.source->name.c_str(), csRef.line);
    return true;
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, DefaultSpell *spell, const DLString &constArguments) const
{
    if (!is_websock(ch)) {
        ch->pecho("Эта крутая фишка доступна только в веб-клиенте.");
        return false;
    }

    Register w = WrapperManager::getThis()->getWrapper(spell);
    if (w.type == Register::NONE)
        return false;
        
    WrapperBase *base = get_wrapper(w.toObject());
    if (!base)
        return false;

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Scripting::IdRef methodId(methodName);
    Register retval = base->getField(methodId);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        std::vector<DLString> parms(2);
        // Create codesource subject.
        parms[0] = dlprintf("spell/%s/%s",
                        spell->getSkill()->getName().c_str(),
                        methodName.c_str());   

        // Create codesource body with example code.
        DLString tmpl;
        if (!findExample(ch, methodName, "spell", tmpl))
            return false;

        tmpl.replaces("@name@", DLString("\"") + spell->getSkill()->getName() + "\"");
        tmpl.replaces("@trig@", methodName);
        parms[1] = tmpl;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для заклинания, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, DefaultAffectHandler *ah, const DLString &constArguments) const
{
    if (!is_websock(ch)) {
        ch->pecho("Эта крутая фишка доступна только в веб-клиенте.");
        return false;
    }

    Register w = WrapperManager::getThis()->getWrapper(ah);
    if (w.type == Register::NONE)
        return false;
        
    WrapperBase *base = get_wrapper(w.toObject());
    if (!base)
        return false;

    DLString args = constArguments;
    DLString methodName = args.getOneArgument();
    Scripting::IdRef methodId(methodName);
    Register retval = base->getField(methodId);

    // Fenia field not found, try to open the editor with trigger example.
    if (retval.type == Register::NONE) {
        std::vector<DLString> parms(2);
        // Create codesource subject.
        parms[0] = dlprintf("affect/%s/%s",
                        ah->getSkill()->getName().c_str(),
                        methodName.c_str());   

        // Create codesource body with example code.
        DLString tmpl;
        if (!findExample(ch, methodName, "affect", tmpl))
            return false;

        tmpl.replaces("@name@", DLString("\"") + ah->getSkill()->getName() + "\"");
        tmpl.replaces("@trig@", methodName);
        parms[1] = tmpl;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для аффекта, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    return editExisting(ch, retval);
}


PluginInitializer<FeniaTriggerLoader> initFeniaTriggerLoader;

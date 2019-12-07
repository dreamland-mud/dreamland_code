#include "feniatriggers.h"
#include "wrapperbase.h"
#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "plugininitializer.h"
#include "wrappermanager.h"
#include "iconvmap.h"
#include "websocketrpc.h"
#include "dlfileloader.h"
#include "pcharacter.h"
#include "room.h"
#include "descriptor.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"

static IconvMap utf2koi("utf-8", "koi8-r");

static const DLString EXAMPLE_FOLDER = "fenia.examples";

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
        buf << t->first << " ";
    }

    buf << endl;
    ch->send_to(buf);
}

static Register get_wrapper_for_index_data(int vnum, const DLString &type)
{
    Register w;
    if (type == "obj") {
        w = WrapperManager::getThis()->getWrapper(get_obj_index(vnum));
    } else if (type == "room") {
        w = WrapperManager::getThis()->getWrapper(get_room_index(vnum));
    } else if (type == "mob") {
        w = WrapperManager::getThis()->getWrapper(get_mob_index(vnum));
    }
    return w;
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, XMLIndexData &indexData, const DLString &constArguments) const
{
    if (!is_websock(ch)) {
        ch->println("Эта крутая фишка доступна только в веб-клиенте.");
        return false;
    }

    Register w = get_wrapper_for_index_data(indexData.getVnum(), indexData.getIndexType());
    if (w.type == Register::NONE)
        return false;
        
    WrapperBase *base = get_wrapper(w.toObject());
    if (!base) {
        ch->println("Не могу найти wrapper base, всё плохо.");
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
            ch->println("Название триггера должно начинаться с 'on' или 'post'.");
            return false;
        }

        if (!stringIsCapitalized(trigType)) {
            ch->println("Название триггера должно выглядеть так: onFight, postSpeech, onWear и т.п.");
            return false;
        }

        // Look up trigger example for given index type and method name.
        IndexTriggers::const_iterator i = indexTriggers.find(indexData.getIndexType());
        if (i == indexTriggers.end()) {
            ch->println("Нет ни одного триггера, попросите богов принять меры.");
            return false;
        }
        TriggerContent::const_iterator t = i->second.find(methodName);
        if (t == i->second.end()) {
            ch->printf("Триггер %s не найден, проверьте написание или попросите богов добавить его.\r\n", methodName.c_str());
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
        DLString tmpl = t->second;
        tmpl.replaces("@vnum@", indexData.getVnum());
        tmpl.replaces("@trig@", methodName);
        parms[1] = tmpl;

        // Open the editor.
        ch->desc->writeWSCommand("cs_edit", parms);
        ch->printf("Запускаю веб-редактор для нового сценария, триггер %s.\r\n", methodName.c_str());
        return true;
    }

    // Fenia field is not a function.
    if (retval.type != Register::FUNCTION) {
        ch->println("Это поле уже задано, но это не функция. Попробуйте что-то еще.");
        return false;
    }

    // Fenia function found, open the editor with the containing codesource.
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


PluginInitializer<FeniaTriggerLoader> initFeniaTriggerLoader;

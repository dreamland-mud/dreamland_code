#include "feniatriggers.h"
#include "wrapperbase.h"
#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "plugininitializer.h"
#include "wrappermanager.h"
#include "iconvmap.h"
#include "dlfileloader.h"
#include "pcharacter.h"
#include "descriptor.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"

static IconvMap utf2koi("utf-8", "koi8-r");

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
    // TODO support mobs and rooms.
    DLFileLoader loader("fenia.examples", "");
    loader.loadAll();

    DLFileLoader::Files::const_iterator f;
    DLFileLoader::Files files = loader.getAll();
    for (f = files.begin(); f != files.end(); f++) {
        // Assume that the source file is in utf-8.
        triggers[f->first] = DLString(utf2koi(f->second.content));
    }
}

void FeniaTriggerLoader::destruction()
{

}

static DLString normalizeTriggerName(const DLString &name) 
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

void FeniaTriggerLoader::showAvailableTriggers(PCharacter *ch) const
{
    ostringstream buf;

    buf << "Доступные триггера on и post: ";
    for (TriggerContent::const_iterator t = triggers.begin(); t != triggers.end(); t++) {
        DLString key = t->first;
        key.upperFirstCharacter();
        buf << key << " ";
    }

    buf << endl;
    ch->send_to(buf);
}

bool FeniaTriggerLoader::openEditor(PCharacter *ch, XMLIndexData &indexData, const DLString &constArguments) const
{
    if (ch->desc->websock.state != WS_ESTABLISHED) {
        ch->println("Эта крутая фишка доступна только в веб-клиенте.");
        return false;
    }

    // TODO, obviously.
    obj_index_data *pObj = get_obj_index(indexData.getVnum());
    if (!pObj)
        return false;

    Register w = WrapperManager::getThis()->getWrapper(pObj);
    if (w.type == Register::NONE) {
        ch->println("Не могу повесить враппер.");
        return false;
    }

    WrapperBase *base = get_wrapper(pObj->wrapper);
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
        DLString methodKey = normalizeTriggerName(methodName);
        if (methodKey == methodName) {
            ch->println("Название триггера должно начинаться с 'on' или 'post'.");
            return false;
        }
        
        DLString savedKey = methodKey;
        methodKey.capitalize();
        if (methodKey.empty() || savedKey != methodKey) {
            ch->println("Название триггера должно выглядеть так: onFight, postSpeech, onWear и т.п.");
            return false;
        }

        TriggerContent::const_iterator t = triggers.find(methodKey.toLower());
        if (t == triggers.end()) {
            ch->printf("Триггер %s не найден, проверьте написание или попросите богов добавить его.\r\n", methodName.c_str());
            return false;
        }

        std::vector<DLString> parms(2);
        // Create codesource subject.
        parms[0] = dlprintf("areas/%s/%s/%d", 
                        indexData.getArea()->area_file->file_name, 
                        indexData.getIndexType(),
                        indexData.getVnum());   

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
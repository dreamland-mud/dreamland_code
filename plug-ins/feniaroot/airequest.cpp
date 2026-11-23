#include <sys/stat.h>

#include "airequest.h"
#include "logstream.h"
#include "iconvmap.h"
#include "dldirectory.h"
#include "dlfilestream.h"
#include "dreamland.h"
#include "feniamanager.h"
#include "register-impl.h"
#include "reglist.h"

static IconvMap ai_koi2utf("koi8-u", "utf-8");

// The worker polls the directory instead of watching close-write, so the file
// is written under mkstemp's name and only then renamed to *.json: the worker
// reads *.json only and never sees half a file.
AiRequestResult ai_request(const DLString &kind, const Json::Value &payload, DLString &error)
{
    static const DLString kindChars = "abcdefghijklmnopqrstuvwxyz0123456789_";
    static const size_t maxBytes = 65536;

    if (kind.empty() || kind.size() > 32 || kind.find_first_not_of(kindChars) != DLString::npos) {
        error = "kind: lowercase latin letters, digits or _, up to 32 characters";
        return AI_REQUEST_INVALID;
    }

    Json::Value body;
    body["kind"] = kind;
    body["time"] = (Json::Int64)dreamland->getCurrentTime();
    body["payload"] = payload;

    Json::FastWriter writer;
    // FastWriter passes KOI8 bytes through untouched, so the whole document
    // converts at once, as jsonservlet.cpp does.
    DLString text = ai_koi2utf(writer.write(body));
    if (text.size() > maxBytes) {
        error = "aiRequest: payload over 64 KB";
        return AI_REQUEST_INVALID;
    }

    try {
        DLDirectory dir(dreamland->getMiscDir(), "ai");
        if (!dir.exist() && ::mkdir(dir.getCPath(), 0755) < 0 && !dir.exist()) {
            error = "cannot create " + dir.getPath();
            LogStream::sendError() << "aiRequest: " << error << endl;
            return AI_REQUEST_IO;
        }

        DLFile tmp = dir.tempEntry();
        DLFileStream(tmp).fromString(text);
        // fromString does not check the write: a full disk leaves a short
        // file, which must not be published as a request.
        if (tmp.getSize() != (int)text.size()) {
            error = "short write to " + tmp.getPath();
            LogStream::sendError() << "aiRequest: " << error << endl;
            tmp.remove();
            return AI_REQUEST_IO;
        }
        if (!tmp.rename(tmp.getPath() + ".json")) {
            error = "cannot rename " + tmp.getPath();
            tmp.remove();
            return AI_REQUEST_IO;
        }

    } catch (const ::Exception &e) {
        error = e.what();
        LogStream::sendError() << "aiRequest: " << error << endl;
        return AI_REQUEST_IO;
    }

    return AI_REQUEST_OK;
}

// The same gate every Fenia feature asks (dreamland_fenia utils/ai): the
// worker's kill switch, 'ai on', 'ai on <feature>' and a fresh heartbeat.
// A missing or broken helper reads as off.
bool ai_enabled(const DLString &feature)
{
    static Scripting::IdRef ID_TMP("tmp"), ID_AI("ai"), ID_ENABLED("enabled");

    if (!FeniaManager::wrapperManager)
        return false;

    try {
        Scripting::Register tmpAi = *(*Scripting::Context::root[ID_TMP])[ID_AI];
        Scripting::RegisterList args;
        args.push_back(Scripting::Register(feature));
        return tmpAi[ID_ENABLED](args).toBoolean();
    }
    catch (const ::Exception &) {
        return false;
    }
}

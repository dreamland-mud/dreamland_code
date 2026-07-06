/* Dreamland trilinguality -- runtime-message translation catalog loader.
 * Wave 3 / Trello 2594.
 *
 * Loads config/translations.json into the process-wide TranslationManager at
 * boot and on every `plug reload most`. Shape:
 *   { "<file>": { "<ru phrase>": { "en": "...", "ua": "..." }, ... }, ... }
 * where <file> is the originating Fenia script path (the key captured by the
 * Fenia `._()` call). Every entry is optional; a missing file/phrase/lang falls
 * back to the RU phrase itself inside TranslationManager::run, so a partial or
 * empty catalog reproduces the historical RU-only output exactly -- which is
 * what makes the roll-out safe and incremental. See translation.h.
 */
#include <jsoncpp/json/json.h>
#include "logstream.h"
#include "configurable.h"
#include "translation.h"

CONFIGURABLE_LOADED(config, translations)
{
    TranslationManager &mgr = TranslationManager::getThis( );
    mgr.clear( );

    for (const auto &fileName: value.getMemberNames( )) {
        const Json::Value &phrases = value[fileName];
        if (!phrases.isObject( ))
            continue;

        for (const auto &ru: phrases.getMemberNames( )) {
            const Json::Value &t = phrases[ru];
            if (!t.isObject( ))
                continue;

            mgr.put( fileName, ru, t["en"].asString( ), t["ua"].asString( ) );
        }
    }

    LogStream::sendNotice( ) << "translations: loaded " << mgr.phraseCount( )
                             << " phrases across " << mgr.fileCount( ) << " files." << endl;
}

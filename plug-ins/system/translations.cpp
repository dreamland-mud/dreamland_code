/* Dreamland trilinguality -- runtime-message translation catalog loader.
 * Wave 3 / Trello 2594.
 *
 * The catalog is SPLIT across many per-subsystem files under
 *   config/translations/<name>.json
 * (e.g. affect.json, skillcommand.json). A single monolithic file grew too big
 * to open in the in-client `fedit` editor (450 KB crashed the web client) and
 * made git diffs / merges painful, so it was broken up. The filename is
 * arbitrary -- only the KEYS inside matter -- so a subsystem that outgrows one
 * file can be split further into extra flat *.json files with no code change.
 *
 * Each shard is registered in configReg (so `fedit translations/<name>` can edit
 * it) and merged into the one process-wide TranslationManager. Shape of every
 * file:
 *   { "<file>": { "<ru phrase>": { "en": "...", "ua": "..." }, ... }, ... }
 * where <file> is the originating Fenia script path (the key captured by the
 * Fenia `._()` call). Every entry is optional; a missing file/phrase/lang falls
 * back to the RU phrase itself inside TranslationManager::run, so a partial or
 * empty catalog reproduces the historical RU-only output exactly -- which is
 * what makes the roll-out safe and incremental. See translation.h.
 *
 * TranslationManager has no remove-by-file, so any (re)load or `fedit` save
 * rebuilds the whole catalog from every registered shard -- cheap (a few
 * thousand puts) and always consistent.
 */
#include <list>
#include <jsoncpp/json/json.h>
#include "logstream.h"
#include "configurable.h"
#include "translation.h"
#include "dldirectory.h"
#include "dlfile.h"
#include "plugininitializer.h"
#include "dreamland.h"

static void rebuildTranslationCatalog( );

/** One config/translations/<name>.json file. Its loaded() hook runs on the
 *  initial Configurable::load() AND on every `fedit` save, so either path keeps
 *  the runtime catalog in sync. */
class TranslationShard : public Configurable {
public:
    typedef ::Pointer<TranslationShard> Pointer;

protected:
    virtual void loaded( ) { rebuildTranslationCatalog( ); }
};

/** Shards currently loaded. Owned here; also referenced by configReg. */
static std::list<TranslationShard::Pointer> translationShards;

static void mergeShard( TranslationManager &mgr, const Json::Value &value )
{
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
}

static void rebuildTranslationCatalog( )
{
    TranslationManager &mgr = TranslationManager::getThis( );
    mgr.clear( );

    for (auto &shard: translationShards)
        mergeShard( mgr, shard->getValue( ) );

    LogStream::sendNotice( ) << "translations: loaded " << mgr.phraseCount( )
                             << " phrases across " << mgr.fileCount( ) << " files ("
                             << translationShards.size( ) << " shards)." << endl;
}

/** Scans config/translations/ at boot and on every `plug reload most`,
 *  (re)creating a TranslationShard per *.json file. */
class TranslationsLoader: public Plugin {
public:
    typedef ::Pointer<TranslationsLoader> Pointer;

    virtual void initialization( )
    {
        // Idempotent: drop any shards from a prior load before re-scanning.
        releaseShards( );

        try {
            DLDirectory dir( dreamland->getTableDir( ), "config/translations" );
            dir.open( );

            for ( ;; ) {
                DLFile local = dir.nextTypedEntry( ".json" ); // "affect.json" -> EOF throws
                DLString name = local.getFileName( );         // "affect"
                if (name.empty( ))
                    continue;

                TranslationShard::Pointer shard( NEW );
                shard->setPath( "config/translations/" + name );
                // push before load() so the rebuild inside load()->loaded() sees it
                translationShards.push_back( shard );
                shard->load( );
            }
        }
        catch (const ExceptionDBIOEOF &) {
            // normal end of directory
        }
        catch (const ExceptionDBIO &ex) {
            LogStream::sendError( ) << "translations loader: " << ex.what( ) << endl;
        }

        // Authoritative final count (also handles an empty/missing directory).
        rebuildTranslationCatalog( );
    }

    virtual void destruction( )
    {
        releaseShards( );
        TranslationManager::getThis( ).clear( );
    }

private:
    void releaseShards( )
    {
        // Unregister every translation shard from configReg. Driven off configReg
        // rather than our own list so a shard leaked by a prior instance (if
        // destruction() didn't run before a reload) is still purged -- otherwise
        // configReg->add (emplace) would silently keep the stale entry.
        if (configReg) {
            for (auto &cfg: configReg->getAll( "config/translations/" ))
                cfg->unload( );
        }
        translationShards.clear( );
    }
};

PluginInitializer<TranslationsLoader> initTranslationsLoader;

/* Externalized per-language flag-table messages (bits.conf Variant 2).
 *
 * Flag tables generated from bits.conf carry value + EN name + a single
 * in-binary RU message (the historical 4th column). User-facing messages in
 * EN/RU/UA -- with grammar-case pads -- live in dreamland_world/config/
 * flagmessages.json and are loaded into this store at boot.
 *
 * FlagTable::message()/messages() consult this store first, keyed by the table
 * name (resolved via FlagTableRegistry) and the flag name, falling back to the
 * in-binary RU message when an entry is absent. An EMPTY store therefore
 * reproduces the historical RU-only output exactly, which is what makes the
 * roll-out safe and incremental.
 *
 * ruffina/kit, 2026 -- closes the "// TODO after l10n" left in flagtable.cpp.
 */
#ifndef FLAGMESSAGESTORE_H
#define FLAGMESSAGESTORE_H

#include <map>
#include <array>
#include "dlstring.h"
#include "lang.h"

class FlagMessageStore {
public:
    static FlagMessageStore & shared( )
    {
        static FlagMessageStore instance;
        return instance;
    }

    bool empty( ) const { return store.empty( ); }
    void clear( ) { store.clear( ); }

    // Store one (table, flag, lang) message pad. No-op for empty pads so a
    // missing JSON key never shadows the in-binary fallback.
    void set( const DLString &table, const DLString &flag, lang_t lang, const DLString &pad )
    {
        if (lang < LANG_MIN || lang >= LANG_MAX || pad.empty( ))
            return;

        store[table][flag][lang] = pad;
    }

    // Return the message pad for (table, flag, lang), or an empty string when
    // not present (caller then falls back to the in-binary message).
    const DLString & get( const DLString &table, const DLString &flag, lang_t lang ) const
    {
        if (lang < LANG_MIN || lang >= LANG_MAX)
            return DLString::emptyString;

        std::map<DLString, FlagMap>::const_iterator t = store.find( table );
        if (t == store.end( ))
            return DLString::emptyString;

        FlagMap::const_iterator f = t->second.find( flag );
        if (f == t->second.end( ))
            return DLString::emptyString;

        return f->second[lang];
    }

private:
    typedef std::array<DLString, LANG_MAX> LangArray;  // indexed by lang_t
    typedef std::map<DLString, LangArray> FlagMap;     // flag name -> per-lang pad
    std::map<DLString, FlagMap> store;                 // table name -> flags
};

#endif

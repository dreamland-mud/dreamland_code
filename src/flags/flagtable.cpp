/* $Id: flagtable.cpp,v 1.1.2.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include "flagtable.h"
#include "flagtableregistry.h"
#include "flagmessagestore.h"
#include "dl_strings.h"
#include "stringlist.h"

/*----------------------------------------------------------------------
 * FlagTable
 *---------------------------------------------------------------------*/

// Single store instance for the whole process, regardless of which shared
// object touches it (the JSON loader lives in a plug-in, the readers here).
FlagMessageStore & FlagMessageStore::shared( )
{
    static FlagMessageStore instance;
    return instance;
}

// Resolve the raw (un-declined) message pad for one field in the requested
// language: the external store (requested lang, then RU) wins, otherwise the
// in-binary RU message, otherwise the bare EN name. With an empty store this
// returns exactly what the historical code did, so behaviour is preserved
// until messages are externalized into config/flagmessages.json.
static DLString resolveMessagePad( const FlagTable *table, const FlagTable::Field &field, lang_t lang )
{
    const FlagMessageStore &store = FlagMessageStore::shared( );

    if (!store.empty( )) {
        const DLString &tableName = FlagTableRegistry::getName( table );
        if (!tableName.empty( )) {
            const DLString &ext = store.get( tableName, field.name, lang );
            if (!ext.empty( ))
                return ext;

            // UA with no own entry falls back to RU before the in-binary message.
            if (lang == LANG_UA) {
                const DLString &ru = store.get( tableName, field.name, LANG_RU );
                if (!ru.empty( ))
                    return ru;
            }
        }
    }

    // EN: the flag's own name is already English, so it IS the EN message unless
    // an explicit "en" override was supplied above. Never fall through to the RU
    // message for an EN viewer, so the JSON only needs to carry "ua".
    if (lang == LANG_EN)
        return field.name;

    // RU, and the ultimate fallback for any language: the in-binary RU message,
    // then the bare name.
    if (field.message)
        return field.message;

    return field.name;
}

int FlagTable::index( const DLString &arg, bool strict ) const
{
    if (arg.empty( ))
        return NO_FLAG;

    // Quick strict search 
    if (strict) {
        for (int i = 0; i < size; i++) 
            if (arg ^ fields[i].name)
                return i;

        return NO_FLAG;
    }

    // Match against all name and message entries in all cases
    // TODO after l10n all cases should be cached
    for (int i = 0; i < size; i++) {
        if (arg.strPrefix( fields[i].name ))
            return i;

        std::list<DLString> cases = russian_cases(fields[i].message);
        for (auto &msg: cases)
            if (arg.strPrefix(msg))
                return i;
    }

    return NO_FLAG;
}

bitnumber_t FlagTable::value( const DLString &arg, bool strict ) const
{
    int i = index( arg, strict );

    if (i == NO_FLAG)
        return NO_FLAG;
    else
        return fields[i].value;
}

bitstring_t FlagTable::bitstring( const DLString &arg, bool strict ) const
{
    Bitstring marked;
    bool found = false;
    DLString args( arg );

    while (!args.empty( )) {
        DLString word = args.getOneArgument( );
        int i = index( word, strict );

        if (i != NO_FLAG) {
            marked.setBitNumber( fields[i].value );
            found = true;
        }
    }
    
    if (found)
        return marked;
    else
        return NO_FLAG;
}

DLString FlagTable::names( bitstring_t bits ) const
{
    if (bits == NO_FLAG)
        return DLString::emptyString;

    DLString buf;
    Bitstring b( bits );
    
    for (int i = 0; i <= max; i++)
        if (b.isSetBitNumber( i ) && reverse[i] != NO_FLAG) {
            if (!buf.empty( ))
                buf << " ";

            buf << fields[reverse[i]].name;
        }

    return buf;
}

DLString FlagTable::messages( bitstring_t bits, bool comma, char gcase, lang_t lang ) const
{
    if (bits == NO_FLAG)
        return DLString::emptyString;

    DLString buf;
    Bitstring b( bits );

    for (int i = 0; i <= max; i++)
        if (b.isSetBitNumber( i ) && reverse[i] != NO_FLAG) {
            if (!buf.empty( ))
                buf << (comma ? ", " : " ");

            buf << resolveMessagePad( this, fields[reverse[i]], lang ).ruscase( gcase );
        }

    return buf;
}

StringList FlagTable::toStringList( bitstring_t bits, char gcase, lang_t lang ) const
{
    StringList result;

    if (bits == NO_FLAG)
        return result;

    Bitstring b( bits );

    for (int i = 0; i <= max; i++)
        if (b.isSetBitNumber( i ) && reverse[i] != NO_FLAG) {
            result.push_back( resolveMessagePad( this, fields[reverse[i]], lang ).ruscase( gcase ) );
        }

    return result;
}

DLString FlagTable::name( bitnumber_t value ) const
{
    if (value < 0 || value > max || reverse[value] == NO_FLAG)
        return DLString::emptyString;
    else
        return fields[reverse[value]].name;
}

DLString FlagTable::message( bitnumber_t value, char gcase, lang_t lang ) const
{
    if (value < 0 || value > max || reverse[value] == NO_FLAG)
        return DLString::emptyString;

    return resolveMessagePad( this, fields[reverse[value]], lang ).ruscase( gcase );
}


